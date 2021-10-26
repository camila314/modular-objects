static bool isRestricted(PlayerObject* th) {
	#if _WIN32
    return th->isFlying() || th->m_isSpider || th->m_isBall;
    #else
    return th->isFlying() || th->_isSpider() || th->_isBall();
    #endif
}

static bool playerTouchesObject(CCRect* playerRect, CCRect* hitboxRect) {
    float playerMaxX = playerRect->getMaxX();
    float playerMinX = playerRect->getMinX();
    float playerMaxY = playerRect->getMaxY();
    float playerMinY = playerRect->getMinY();

    float objMinX = hitboxRect->getMinX();
    float objMaxX = hitboxRect->getMaxX();
    float objMinY = hitboxRect->getMinY();
    float objMaxY = hitboxRect->getMaxY();

    return (objMinX <= playerMinX && objMaxX <= playerMaxX && objMinY <= playerMinY && objMaxY <= playerMaxY);

}

void ObjectManager::hooks::checkGameObjectTypeCollisions(PlayLayer* self, PlayerObject* player, float delta) {
    
    player->preCollision();

    #if __APPLE__
    float groundHeight = player->_groundHeight();
    float vehicleSize = player->_vehicleSize();
    auto sections = self->m_sections;
    bool isUpsideDown = player->_isUpsideDown();
    #else
    float groundHeight = player->m_groundHeight;
    float vehicleSize = player->m_vehicleSize;
    auto sections = self->m_pObjectContainerArrays;
    bool isUpsideDown = player->m_isUpsideDown;
    #endif

    float cubeSizeScalar;
    if (vehicleSize == 1.0) {
            cubeSizeScalar = 0.0;
    }
    else {
            cubeSizeScalar = (1.0 - vehicleSize) * groundHeight * 0.5;
    }
    groundHeight /= 2;
    float groundY = 90.0 + groundHeight - cubeSizeScalar;

    float ypos = player->getPositionY();
    if (player->getPositionY() >= groundY || isRestricted(player) || self->dualMode || !isUpsideDown) {
        if (ypos > 2790.0 + cubeSizeScalar) {
            return;
        }
        if(!isUpsideDown && !player->isRising) {
            player->setPositionY(groundY);
            player->hitGround(false);
            player->updateCollide(player, true);
            player->isOnSlope2 = false;
        }

        r14 = !player->isFlying() && !player->isBall && self->dualMode;

        if (player->isRestricted() || self->dualMode) {

            float ceiling;
            float ground;
            if (self->dualMode != 0x0) {
                    CCSize wSize = CCDirector::sharedDirector()->getWinSize();

                    groundPos = self->groundLayer2->getPosition();
                    float difference = self->groundLayer1->getGroundY() - groundPos.y;

                    ceiling = self->cameraY + wSize.height - difference + 1;
                    if (self->dualMode != 0x0) {
                            float pos = self->groundLayer1->getPosition();
                            ground = self->groundLayer1->getGroundY() + pos.y + self->cameraY - 1;
                    }
            } else {
                    ceiling = self->ceilingRestrictionY;
                    ground = self->groundRestrictionY;
            }
            float localGround = self->groundRestrictionY;
            float restrictedSpace = ceiling - groundHeight + cubeSizeScalar;

            if (player->getPositionY() > restrictedSpace) {
                if (player->isOnSlope2 && player->getPositionY() > ceiling + cubeSizeScalar) {
                    self->destroyPlayer(player, nullptr);
                    return;
                } else {
                    bool shouldPush = false;
                    if (!player->isFlying() && !player->isBall && self->dualMode && !isUpsideDown) {
                        shouldPushn = true;
                        if (!player->isSafeFlip(0.2)) {
                            if (!player->isSafeMode(0.2)) {
                                self->destroyPlayer(player, nullptr);
                                return;
                            }
                        }
                    }

                    player->setPositionY(restrictedSpace);

                    if (shouldPush) {
                        player->pushDown();
                    } else if (player->yAccel > 0) {
                        player->hitGround(!isUpsideDown() || -5.0 > ceiling - self->ceilingRestrictionY);
                    }

                    player->updateCollide(isUpsideDown, 0);

                    if (-5.0 > ceiling - self->ceilingRestrictionY) {
                            if (player->yAccel >= 0) {
                                    player->specialGroundHit();
                            }
                    }
                    player->isOnSlope2 = false;
                }
            } else {
                float relativeGround = groundHeight + groundY - cubeSizeScalar;
                if (relativeGround > player->getPositionY()) {
                    if (player->isOnSlope2 && groundY - cubeSizeScalar > player->getPositionY()) {
                        self->destroyPlayer(player, nullptr);
                        return;
                    }
                    bool shouldPush;
                    if (!player->isFlying() && !player->isBall && self->dualMode && isUpsideDown) {
                        if (!player->isSafeFlip(0.2) && !player->isSafeMode(0.2)) {
                            self->destroyPlayer(player, nullptr);
                            return;
                        }
                        shouldPush = true;
                    } else {
                        shouldPush = false;
                    }

                    player->setPositionY(relativeGround);
                    if (shouldPush) {
                        player->pushDown(player);
                    } else {
                        player->hitGround(ground- self->groundRestrictionY > 0.5 || isUpsideDown);
                    }
                    player->updateCollide(!isUpsideDown, 0);
                    if (ground - self->groundRestrictionY > 0.5) {
                            if (player->yAccel <= 0) {
                                    player->specialGroundHit();
                            }
                    }
                    player->isOnSlope2 = false;
                }
            }
        }

        CCRect playerRect = *player->getObjectRect();

        int section = self->sectionForPos(player->getPositionX());

        CCArray* sect;
        for (int index = section-1; index <= section; index++) {

            if (0 > index >= sections->count()) {
                CCArray* sect = sections->objectAtIndex(index);
                if (sect) {
                    if (sect->count() > 0) {
                    	CCObject* gameObj_;
                        CCARRAY_FOREACH(sect, gameObj_) {
                        	GameObject* gameObj = reinterpret_cast<GameObject*>(gameObj_);


                            int objectType = gameObj->objectType;
                            if (!gameObj->toggledOff && !gameObj->isUnloaded && (objectType <= 39 || !gameObj->hasBeenActivatedByPlayer(player))) {
                                switch (objectType) {
                                    case 0:
                                    case kGameObjectTypeSolid: // case 20
                                        self->touchedSurfaces->addObject(gameObj);
                                        break;
                                    case kGameObjectTypeHazard: // case 2 
                                        self->touchedHazards->addObject(gameObj);
                                        break;
                                    case 7:
                                    case 39: 
                                        break;
                                    default: 
                                        if (!gameObj->hasBeenActivatedByPlayer(player)) {
                                            auto w = CCRect(0.0,0.0,0.0,0.0);
                                            cocos2d::CCRect& hitboxRect = w;
                                            if (objectType == 25) {
                                                hitboxRect = gameObj->getObjectRect(2., 2.);
                                            } else {
                                                hitboxRect = gameObj->getObjectRect();
                                            }

                                            bool didCollide = playerTouchesObject(playerRect, hiboxRect);

                                            if ( (didCollide && gameObj->hitboxSize > 0) || self->objectIntersectsCircle(player, gameObj)) {
                                                bool isTrueCollision = true;
                                                if (gameObj->usesOrientedBox) {
                                                    OBB2D* oldBox = gameObj->getOrientedBox();
                                                    gameObj->updateOrientedBox();
                                                    OBB2D* newBox = gameObj->getOrientedBox();
                                                    isTrueCollision= oldBox->overlaps1Way(newBox);
                                                }
                                                if (objectType == 25) {
                                                    hitboxRect = gameObj->getObjectRect();
                                                }
                                                if (isTrueCollision) {
                                                    switch (objectType) {
                                                        case kGameObjectTypeInverseGravityPortal:
                                                            if (!isUpsideDown) {
                                                                    self->playGravityEffect(true);
                                                            }
                                                            player->portalCircleLocation = gameObj->getPosition();
                                                            player->portalCircleObject = gameObj;
                                                            self->flipGravity(player, true, false);
                                                            gameObj->playShineEffect(r13);
                                                            gameObj->activatedByPlayer(player);
                                                            break;
                                                        case kGameObjectTypeNormalGravityPortal:
                                                            if (isUpsideDown) {
                                                                    self->playGravityEffect(false);
                                                            }
                                                            player->portalCircleLocation = gameObj->getPosition();
                                                            player->portalCircleObject = gameObj;
                                                            self->flipGravity(player, false, false);
                                                            gameObj->playShineEffect();
                                                            gameObj->activatedByPlayer(player);
                                                            break;
                                                        case kGameObjectTypeShipPortal:
                                                            self->playerWillSwitchMode(player, gameObj);
                                                            self->switchToFlyMode(player, gameObj, false, 5);
                                                            gameObj->playShineEffect();
                                                            gameObj->activatedByPlayer(player);
                                                            break;
                                                        case kGameObjectTypeCubePortal:
                                                            self->playerWillSwitchMode(player, gameObj);
                                                            player->portalCircleLocation = gameObj->getPosition();
                                                            player->portalCircleObject = gameObj;
                                                            self->willSwitchToMode(false, player);
                                                            player->modeDidChange();
                                                            gameObj->playShineEffect();
                                                            gameObj->activatedByPlayer(player);
                                                            break;
                                                        case kGameObjectTypeYellowJumpPad:
                                                        case kGameObjectTypePurpleJumpPad:
                                                        case kGameObjectTypeRedJumpPad:
                                                            self->bumpPlayer(player, gameObj);
                                                            break;
                                                        case kGameObjectTypeBlueJumpPad:
                                                            if (isUpsideDown ^ !local_isPadUpsideDown(gameObj)) {
                                                                bool newDirection =  !isUpsideDown;
                                                                self->playGravityEffect(newDirection);

                                                                CCPoint objPosition = gameObj->getPosition();
                                                                objPosition.y -= 10;
                                                                player->portalCircleLocation = objPosition;
                                                                player->portalCircleObject = gameObj;

                                                                gameObj->activatedByPlayer(player);
                                                                player->propellPlayer(0.8);
                                                                self->flipFravity(player, newDirection, true);
                                                                player->unused_628 = true;
                                                            }
                                                            break;
                                                        case kGameObjectTypeYellowJumpRing:
                                                        case kGameObjectTypePinkJumpRing:
                                                        case kGameObjectTypeBlueJumpRing:
                                                        case kGameObjectTypeRedJumpRing:
                                                            self->playerTouchedRing(player, gameObj);
                                                            break;
                                                        case kGameObjectTypeInverseMirrorPortal:
                                                            player->portalCircleLocation = gameObj->getPosition();
                                                            player->portalCircleObject = gameObj;
                                                            self->toggleFlipped(true, false);
                                                            gameObj->playShineEffect();
                                                            gameObj->triggerActivated(0.0);
                                                        case kGameObjectTypeNormalMirrorPortal:
                                                            player->portalCircleLocation = gameObj->getPosition();
                                                            player->portalCircleObject = gameObj;
                                                            self->toggleFlipped(false, false);
                                                            gameObj->playShineEffect();
                                                            gameObj->triggerActivated(0.0);
                                                        case kGameObjectTypeBallPortal:
                                                            self->playerWillSwitchMode(player, gameObj);
                                                            self->willSwitchToMode(16, player);
                                                            if (self->dualMode) {
                                                                if (self->dualObject) {
                                                                        gameObj = self->dualObject;
                                                                }
                                                            }
                                                            player->portalCircleLocation = gameObj->getPosition();
                                                            player->portalCircleObject = gameObj;
                                                            self->latestVehicleObject = gameObj;

                                                            player->toggleRollMode(true);
                                                            gameObj->playShineEffect();
                                                            gameObj->activatedByPlayer(player);
                                                        case kGameObjectTypeMiniSizePortal:
                                                            player->portalCircleLocation = gameObj->getPosition();
                                                            player->portalCircleObject = gameObj;
                                                            player->togglePlayerScale(false);
                                                            gameObj->playShineEffect();
                                                            gameObj->activatedByPlayer(player);
                                                        case kGameObjectTypeRegularSizePortal:
                                                            player->portalCircleLocation = gameObj->getPosition();
                                                            player->portalCircleObject = gameObj;
                                                            player->togglePlayerScale(true);
                                                            gameObj->playShineEffect();
                                                            gameObj->activatedByPlayer(player);
                                                        case kGameObjectTypeUfoPortal:
                                                            self->playerWillSwitchMode(player, gameObj);
                                                            self->switchToFlyMode(player, gameObj, false, 19);
                                                            gameObj->playShineEffect();
                                                            gameObj->activatedByPlayer(player);
                                                            break;
                                                        case kGameObjectTypeTrigger:
                                                            if (gameObj->touchTriggered) {
                                                                if (!self->effectManager->hasBeenTriggered(gameObj->uniqueID)) {
                                                                        self->effectManager->storeTriggeredID(gameObj->uniqueID);
                                                                        gameObj->triggerObject(self);
                                                                }
                                                                gameObj->triggerActivated(0.0);
                                                            }
                                                        case kGameObjectTypeUserCoin:
                                                        case kGameObjectTypeSecretCoin:
                                                            if (!self->practiceMode) {
                                                                gameObj->triggerObject(self);
                                                                gameObj->triggerActivated(0.0);
                                                                gameObj->destroyObject();
                                                                if (!self->hasUniqueCoin(gameObj)) {
                                                                    self->pickupItem(gameObj);
                                                                }
                                                            }
                                                            break;
                                                        case kGameObjectTypeDualPortal:
                                                            player->portalCircleLocation = gameObj->getPosition();
                                                            player->portalCircleObject = gameObj;
                                                            self->player2->hasRespawned = true;
                                                            self->toggleDualMode(gameObj, true, player, false);
                                                            self->player2->hasRespawnd = false;
                                                            gameObj->playShineEffect();
                                                            gameObj->triggerActivated(0.0);
                                                            break;
                                                        case kGameObjectTypeSoloPortal:
                                                            player->portalCircleLocation = gameObj->getPosition();
                                                            player->portalCircleObject = gameObj;
                                                            self->toggleDualMode(gameObj, false, player, false);
                                                            gameObj->playShineEffect();
                                                            gameObj->triggerActivated(0.0);
                                                            break;
                                                        case kGameObjectTypeSlope:
                                                            player->collidedWithSlope(delta, gameObj, false);
                                                            /*CCRect pRect = player->getObjectRect();
                                                            playerMaxX = pRect->getMaxX();
                                                            playerMinX = pRect->getMinX();
                                                            playerMaxY = pRect->getMaxY();
                                                            playerMinY = pRect->getMinY();*/
                                                            break;
                                                        case kGameObjectTypeWavePortal:
                                                            self->playerWillSwitchMode(player, gameObj);
                                                            self->switchToFlyMode(player, gameObj, false, 26);
                                                            gameObj->playShineEffect();
                                                            gameObj->activatedByPlayer(player);
                                                            break;
                                                        case kGameObjectTypeRobotPortal:
                                                            self->playerWillSwitchMode(player, gameObj);
                                                            self->willSwitchToMode(27, player);
                                                            if (self->dualMode) {
                                                                if (self->dualObject) {
                                                                    gameObj = self->dualObject;
                                                                }
                                                            }
                                                            player->portalCircleLocation = gameObj->getPosition();
                                                            player->portalCircleObject = gameObj;
                                                            self->latestVehicleObject = gameObj;

                                                            player->toggleRobotMode(true);
                                                            gameObj->playShineEffect();
                                                            gameObj->activatedByPlayer(player);
                                                            break;
                                                        case kGameObjectTypeTeleportPortal:
                                                            player->portalCircleLocation = gameObj->getPosition();
                                                            player->portalCircleObject = gameObj;
                                                            float newY = gameObj->getRealPosition().y + gameObj->teleportY;
                                                            player->setPositionY(newY)
                                                            gameObj->playShineEffect();
                                                            gameObj->activatedByPlayer(player);
                                                            player->spawnPortalCircle(ccc3(255, 255, 0), 50.0);

                                                            GameObject* destObj = gameObj->teleportDestObject;
                                                            if (destObj) {
                                                                    player->portalCircleLocation = destObj->getPosition();
                                                                    player->portalCircleObject = destObj;
                                                                    destObj->playShineEffect();
                                                                    player->spawnPortalCircle(ccc3(0, 200, 255), 50.0);
                                                                    player->resetStreak();
                                                                    if (player->isDart) {
                                                                        player->addStreakGameObjectTypePoint();
                                                                    }
                                                                    self->lightningFlash(gameObj->getPosition(), destObj->getPosition(), player->glowColor, 4.0, 0.2, 100, true, 1.0);
                                                            }
                                                            float something = 60.0;
                                                            if (gameObj->tintGround) {
                                                                    self->cameraSmoothness = 0.5;
                                                                    something = 180.0;
                                                            }
                                                            CCSize winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();

                                                            player->playerTeleported();
                                                            if (player->getPositionY() <= self->cameraY + something + winSize.y && self->cameraY - player->getPositionY() > something) {
                                                                self->disableCameraSmoothness = true;
                                                            }
                                                            break;
                                                        case kGameObjectTypeCollectible:
                                                            if (!self->effectManager->hasBeenTriggered(gameObj->uniqueID)) {
                                                                self->effectManager->storeTriggeredID(gameObj->uniqueID);
                                                                gameObj->triggerObject(self);
                                                            }
                                                            gameObj->triggerObject(self);
                                                            gameObj->triggerActivated(0.0);
                                                            gameObj->destroyObject();
                                                            break;
                                                        case kGameObjectTypeSpiderPortal:
                                                            self->playerWillSwitchMode(player, gameObj);
                                                            self->willSwitchToMode(33, player);
                                                            if (self->dualMode) {
                                                                if (self->dualObject) {
                                                                    gameObj = self->dualObject;
                                                                }
                                                            }
                                                            player->portalCircleLocation = gameObj->getPosition();
                                                            player->portalCircleObject = gameObj;
                                                            self->latestVehicleObject = gameObj;

                                                            player->toggleSpiderMode(true);
                                                            gameObj->playShineEffect();
                                                            gameObj->activatedByPlayer(player);
                                                            break;
                                                    }
                                                }
                                            }
                                        }
                                        break;
                                }
                            }
                        }
                    }
                }
            }
        }

        GameObject* loopObject;
        CCARRAY_FOREACH(loopObject, self->touchedSurfaces) {
            if (!loopObject->getGroupDisabled()) {
                    if (playerTouchesObject(player->getObjectRect(), loopObject->getObjectRect())) {
                        player->collidedWithObject(delta, loopObject);
                    }
            }
        }

        playerRect = player->getObjectRect();
        player->updateOrientedBox();

        CCARRAY_FOREACH(loopObject, self->touchedHazards) {
            if (!loopObject->getGroupDisabled()) {
                if ( (playerTouchesObject(playerRect, loopObject->getObjectRect()) && loopObject->hitboxRect <= 0.0) || self->objectIntersectsCircle(player, loopObject)) {

                    OBB2D* objBox = loopObject->getOrientedBox();
                    OBB2D* playerBox = player->getOrientedBox();
                    if (!loopObject->usesOrientedBox || (objBox->overlaps1Way(playerBox) && playerBox->overlaps1Way(objBox)) && !self->collisionsDisabled) {
                        self->destroyPlayer(player, loopObject);
                        return;
                    }
                }
            }
        }
        self->touchedSurfaces->removeAllObjects();
        self->touchedHazards->removeAllObjects();
        player->postCollision(delta);
        return;
    } else if (!player->isSafeFlip()) {
        self->destroyPlayer(player, nullptr);
        return;
    } else {
        player->setPositionY(groundY);
        player->hitGround(true);
        player->updateCollide(false, 0);
        player->isSliding = false;
        player->isOnSlope2 = false;
        return;
    }
}
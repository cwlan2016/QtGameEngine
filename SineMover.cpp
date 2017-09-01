#include "SineMover.h"
#include <QTimer>
#include "Utilities.h"
#include <cassert>
#include <QtMath>

SineMover::SineMover(Entity *entity):
    Mover(entity),
    moveTimer_(new QTimer(this)),
    amplitude_(20),
    wavelength_(100),
    targetPos_(),
    startPos_(),
    stepSize_(1),
    speed_(200),
    range_(0),
    faceTarget_(true),
    distanceMoved_(0)
{
    // empty
}

/// Moves the Entity in a sine like fashion from its current position to the
/// specified position.
void SineMover::moveEntity_(const QPointF& pos)
{
    Entity* theEntity = entity();

    assert(theEntity != nullptr); // guard: make sure entity isn't dead

    stopMovingEntity();  // stop moving (just in case already moving)

    // update internal variables
    targetPos_ = pos;
    startPos_ = theEntity->pos();

    // calculate range (so we know when we have gone past the target)
    QLineF line(theEntity->pos(),pos);
    range_ = line.length();

    // face target position (if option enabled)
    if (faceTarget_){
        theEntity->setFacingAngle(-1 * line.angle());
    }

    // start moving
    //connect(moveTimer_,&QTimer::timeout,this,&SineMover::onMoveStep_);
    moveTimer_->start(secondsToMs(frequency(stepSize_,speed_)));
}

bool SineMover::faceTarget()
{
    return faceTarget_;
}

void SineMover::setSpeed(int speed)
{
    speed_ = speed;
}

int SineMover::speed()
{
    return speed_;
}

void SineMover::setStepSize(int stepSize)
{
    stepSize_ = stepSize;
}

int SineMover::stepSize()
{
    return stepSize_;
}

void SineMover::setFaceTarget(bool tf)
{
    faceTarget_ = tf;
}

/// Executed periodically to move the controlled entity to the next step of its movement.
void SineMover::onMoveStep_()
{
    Entity* theEntity = entity();

    // if entity has died, stop moving
    if (theEntity == nullptr){
        stopMovingEntity();
        return;
    }

    distanceMoved_ += stepSize_;

    QLineF startToTargetLine(startPos_,targetPos_);

    // calculate horizontal component
    QLineF hComponent(startToTargetLine);
    hComponent.setLength(distanceMoved_);

    // calculate vertical component
    // determine y offset at current xPos
    // y(x) = Acos(kx), A = amplitude, k = 2pi/wavelength
    double PI = 3.14159;
    double yOffset = amplitude_ * qSin(2*PI*distanceMoved_/wavelength_);

    QLineF vComponent(hComponent.p2(),QPointF(0,0));
    vComponent.setAngle(hComponent.angle());
    vComponent.setLength(qAbs(yOffset));

    if (yOffset > 0)
        vComponent.setAngle(vComponent.angle()+90);
    else
        vComponent.setAngle(vComponent.angle()-90);

    // move to final location
    theEntity->setPos(vComponent.p2());

    // if moved far enough
    if (distanceMoved_ > range_){
        emit entitySuccesfullyMoved(this);
        stopMovingEntity();
    }
}

/// Executed whenever the MoveBehavior is asked to stop moving the Entity.
void SineMover::stopMovingEntity_()
{
    moveTimer_->disconnect();
    distanceMoved_ = 0;
}

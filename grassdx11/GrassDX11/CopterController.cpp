#include "CopterController.h"


CopterController::CopterController(void)
{
   transform = XMMatrixIdentity();

   position = create(0, 20, 0);
   velocity = create(0, 0, 0);
   acceleration = create(0, 0, 0);

   transform = XMMatrixTranslationFromVector(position);
}


void CopterController::OnForward(void)
{
   input = FORWARD;
}


void CopterController::OnBackward(void)
{
   input = BACKWARD;
}


void CopterController::OnLeft(void)
{
   input = LEFT;
}


void CopterController::OnRight(void)
{
   input = RIGHT;
}

void CopterController::RotLeft(void)
{
   input = R_LEFT;
}

void CopterController::RotRight(void)
{
   input = R_RIGHT;
}

void CopterController::OnNothing(void)
{
   input = NONE;
}


void CopterController::OnTorque(void)
{
   torque += 0.1 / (1 / 1 - torque);
}


void CopterController::OnDetorque(void)
{
   torque *= 0.9;
}


void CopterController::UpdatePhysics(void)
{
   const float dt = 0.2;

   UpdateInput();

   transform = XMMatrixIdentity();
   transform *= XMMatrixRotationZ(pitch);
   transform *= XMMatrixRotationX(roll);
   transform *= XMMatrixRotationY(yaw);
   
   // TODO: move this, then fan became a part of copter
   XMVECTOR scale, pos, quatr;
   XMMatrixDecompose(&scale, &quatr, &pos, transform);
   XMMATRIX rot = XMMatrixRotationQuaternion(quatr);
   XMVECTOR down = create(0, -1, 0);
   XMVECTOR dir = XMVector3Transform(down, rot) * (-1);
   //

   XMVECTOR mg = create(0, -1, 0);
   acceleration = dir + mg;  // TODO: add torque
   sety(acceleration, 0);

   velocity += acceleration * dt;
   position += velocity * dt;

   transform *= XMMatrixTranslationFromVector(position);

   // Dumps
   pitch *= 0.9;
   roll *= 0.9;
   velocity *= 0.9;

   if (gety(position) < 10) {
      sety(position, 10);
   }
}


void CopterController::UpdateCamera (void)
{
   XMVECTOR toCam = create(-30, 15, 0);
   XMMATRIX rot = XMMatrixRotationY(yaw);
   toCam = XMVector3Transform(toCam, rot);

   cam->SetViewParams(position + toCam, position);
}


void CopterController::UpdateInput (void)
{
   const float angleStep = 0.1;

   switch (input)
   {
   case FORWARD:
      pitch -= angleStep;
      break;
   case BACKWARD:
      pitch += angleStep;
      break;
   case LEFT:
      roll += angleStep;
      break;
   case RIGHT:
      roll -= angleStep;
      break;
   case R_LEFT:
      yaw -= angleStep;
      break;
   case R_RIGHT:
      yaw += angleStep;
      break;
   case NONE:
      break;
   default:
      break;
   }
}

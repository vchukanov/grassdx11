#include "CopterController.h"


CopterController::CopterController(void)
{
   transform = XMMatrixIdentity();

   position = create(-20, 20, 200);
   //position = create(-500, 20, -500);
   //position = create(0, 20, 0);
   velocity = create(0, 0, 0);
   acceleration = create(0, 0, 0);

   transform = XMMatrixTranslationFromVector(position);
}


void CopterController::UpdatePhysics(void)
{
   const float dt = 0.5;

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
   acceleration = dir + mg; 
   sety(acceleration, torque);

   velocity += acceleration * dt;
   position += velocity * dt;

   transform *= XMMatrixTranslationFromVector(position);

   // Dumps
   pitch *= 0.9;
   roll *= 0.9;
   velocity *= 0.9;
   torque = 0;

   if (gety(position) < 10) {
      sety(position, 10);
   }
}


void CopterController::UpdateCamera (void)
{
   XMVECTOR toCam = create(-30, 15, 0);
   XMMATRIX rot = XMMatrixRotationY(yaw);
   toCam = XMVector3Transform(toCam, rot);

   if (!fixCam)
      cam->SetViewParams(position + toCam, position);
}


void CopterController::UpdateInput (void)
{
   const float angleStep = 0.04f;

   if (forward == FORWARD) {
      pitch -= angleStep;
   }

   if (backward == BACKWARD) {
      pitch += angleStep;
   }

   if (left == LEFT) {
      roll += angleStep;
   }

   if (right == RIGHT) {
      roll -= angleStep;
   }

   if (rLeft == R_LEFT) {
      yaw -= angleStep;
   }

   if (rRight == R_RIGHT) {
      yaw += angleStep;
   }

   if (up == UP) {
      torque += 0.4f;
   }

   if (down == DOWN) {
       torque -= 0.4f;
   }
}

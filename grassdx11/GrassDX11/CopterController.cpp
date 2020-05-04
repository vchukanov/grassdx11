#include "CopterController.h"
#include "Copter.h"

CopterController::CopterController (void)
{
   transform = XMMatrixIdentity();

   position = create(0, 20, 0);
   velocity = create(0, 0, 0);
   acceleration = create(0, 0, 0);

   transform = XMMatrixTranslationFromVector(position);
}


void CopterController::InitHeightCtrl (Terrain* terr, float grassR, float heightSc)
{
   terrain = terr;
   grassRadius = grassR;
   heightScale = heightSc;
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

   // correct height
   TerrainHeightData* pHD = terrain->HeightDataPtr();
   float2 vTexCoord = create(getx(position) / grassRadius * 0.5f + 0.5f, getz(position) / grassRadius * 0.5f + 0.5f);

   float terrain_height = pHD->GetHeight(getx(vTexCoord), gety(vTexCoord)) * heightScale;

   if (gety(position) < terrain_height + copter->scale * 2.5) {
      sety(position, terrain_height + copter->scale * 2.5);
   }

   // calc transform
   transform *= XMMatrixTranslationFromVector(position);

   // Dumps
   pitch *= 0.9;
   roll *= 0.9;
   velocity *= 0.9;
   torque = 0;
}


void CopterController::UpdateCamera (void)
{
   XMVECTOR toCam = create(-30, 15, 0);
   XMMATRIX rot = XMMatrixRotationY(yaw);
   toCam = XMVector3Transform(toCam, rot);

   if (fixCam)
      cam->SetViewParams(position + toCam, position);
}


void CopterController::UpdateInput (void)
{
   const float angleStep = 0.1;

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
      torque += 1;
   }

   if (down == DOWN) {
      torque -= 1;
   }
}

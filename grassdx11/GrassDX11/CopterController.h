#pragma once
#include "includes.h"

#include "camera.h"


class CopterController {
public:
   enum Input {
      // controls roll (x) and pitch (y)
      FORWARD,
      BACKWARD,
      LEFT,
      RIGHT,

      R_LEFT,
      R_RIGHT,

      NONE
   };

public:
   CopterController (void);

   void SetupCamera (CFirstPersonCamera *cam_) { cam = cam_; };

   // controls roll (x) and pitch (y)
   void OnForward  (void);
   void OnBackward (void);
   void OnLeft     (void);
   void OnRight    (void);
   
   // controls yaw (z)
   void RotLeft  (void);
   void RotRight (void);


   void OnTorque   (void);
   void OnDetorque (void);

   void OnNothing (void);

   void UpdatePhysics (void);
   void UpdateCamera  (void);

private:
   void UpdateInput (void);

public:
   const float copteerMass = 1;

   float3 position;
   float3 velocity;
   float3 acceleration;

   Input input = NONE;

   float torque = 0.5;  //from 0 to 1
   float roll = 0;
   float pitch = 0;
   float yaw = 0;

public:
   float4x4            transform;
   CFirstPersonCamera *cam;
};
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
 
      UP,
      DOWN,

      NONE
   };

public:
   CopterController (void);

   void SetupCamera (CFirstPersonCamera *cam_) { cam = cam_; };

   // controls roll (x) and pitch (y)
   void OnForward  (void) { forward = FORWARD; }
   void OnBackward (void) { backward = BACKWARD; }
   void OnLeft     (void) { left = LEFT; }
   void OnRight    (void) { right = RIGHT; }
   
   void NonForward  (void) { forward = NONE; }
   void NonBackward (void) { backward = NONE; }
   void NonLeft     (void) { left = NONE; }
   void NonRight    (void) { right = NONE; }
   
   // controls yaw (z)
   void OnRLeft  (void) { rLeft = R_LEFT; }
   void OnRRight (void) { rRight = R_RIGHT; }
   
   void NonRLeft  (void) { rLeft = NONE; }
   void NonRRight (void) { rRight = NONE; }

   // controls height (torque)
   void OnTorque   (void) { up = UP; }
   void OnDetorque (void) { down = DOWN; }

   void NonTorque   (void) { up = NONE; }
   void NonDetorque (void) { down = NONE; }

   void UpdatePhysics (void);
   void UpdateCamera  (void);

   void ToggleFixCam (void) { fixCam = !fixCam; }

private:
   void UpdateInput (void);

public:
   const float copteerMass = 1;

   float3 position;
   float3 velocity;
   float3 acceleration;

   Input left  = NONE;
   Input right = NONE;
   Input forward = NONE;
   Input backward = NONE;

   Input rLeft = NONE;
   Input rRight = NONE;

   Input up = NONE;
   Input down = NONE;

   float torque = 0; 
   float roll = 0;
   float pitch = 0;
   float yaw = 0;

   bool fixCam = true;

public:
   float4x4            transform;
   CFirstPersonCamera *cam;
};
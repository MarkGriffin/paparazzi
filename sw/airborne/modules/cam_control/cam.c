/*
 * $Id$
 *
 * Copyright (C) 2003  Pascal Brisset, Antoine Drouin
 *
 * This file is part of paparazzi.
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */
/** \file cam.c
 *  \brief Pan/Tilt camera library
 *
 */

#include <math.h>
#include "cam.h"
#include "common_nav.h"
#include "autopilot.h"
#include "generated/flight_plan.h"
#include "estimator.h"
#include "traffic_info.h"
#ifdef POINT_CAM
#include "point.h"
#endif // POINT_CAM

#ifdef TEST_CAM
float test_cam_estimator_x;
float test_cam_estimator_y;
float test_cam_estimator_z;
float test_cam_estimator_phi;
float test_cam_estimator_theta;
float test_cam_estimator_hspeed_dir;
#endif // TEST_CAM

#ifdef CAM_PAN_NEUTRAL
#if (CAM_PAN_MAX == CAM_PAN_NEUTRAL)
#error CAM_PAN_MAX has to be different from CAM_PAN_NEUTRAL
#endif
#if (CAM_PAN_NEUTRAL == CAM_PAN_MIN)
#error CAM_PAN_MIN has to be different from CAM_PAN_NEUTRAL
#endif
#endif

#ifdef CAM_TILT_NEUTRAL
#if (CAM_TILT_MAX == CAM_TILT_NEUTRAL)
#error CAM_TILT_MAX has to be different from CAM_TILT_NEUTRAL
#endif
#if (CAM_TILT_NEUTRAL == CAM_TILT_MIN)
#error CAM_TILT_MIN has to be different from CAM_TILT_NEUTRAL
#endif
#endif

#define MIN_PPRZ_CAM ((int16_t)(MAX_PPRZ * 0.05))
#define DELTA_ALPHA 0.2

#ifdef CAM_PAN0
float cam_pan_c = RadOfDeg(CAM_PAN0);
#else
float cam_pan_c;
#endif

#ifdef CAM_TILT0
float cam_tilt_c = RadOfDeg(CAM_TILT0);
#else
float cam_tilt_c;
#endif

float cam_phi_c;
float cam_theta_c;

float cam_target_x, cam_target_y, cam_target_alt;
uint8_t cam_target_wp;
uint8_t cam_target_ac;

uint8_t cam_mode;

int16_t cam_pan_command;
int16_t cam_tilt_command;

void cam_nadir(void);
void cam_angles(void);
void cam_target(void);
void cam_waypoint_target(void);
void cam_ac_target(void);

void cam_init( void ) {
  cam_mode = CAM_MODE_OFF;
}

void cam_periodic( void ) {
  switch (cam_mode) {
  case CAM_MODE_OFF:
    break;
  case CAM_MODE_ANGLES:
    cam_angles();
    break;
  case CAM_MODE_NADIR:
    cam_nadir();
    break;
  case CAM_MODE_XY_TARGET:
    cam_target();
    break;
  case CAM_MODE_WP_TARGET:
    cam_waypoint_target();
    break;
  case CAM_MODE_AC_TARGET:
    cam_ac_target();
    break;
  }
}

/** Computes the servo values from cam_pan_c and cam_tilt_c */
void cam_angles( void ) {
  float cam_pan = 0;
  float cam_tilt = 0;

#ifdef CAM_PAN_NEUTRAL
  float pan_diff = cam_pan_c - RadOfDeg(CAM_PAN_NEUTRAL);
  if (pan_diff > 0)
    cam_pan = MAX_PPRZ * (pan_diff / (RadOfDeg(CAM_PAN_MAX - CAM_PAN_NEUTRAL)));
  else
    cam_pan = MIN_PPRZ * (pan_diff / (RadOfDeg(CAM_PAN_MIN - CAM_PAN_NEUTRAL)));
#endif

#ifdef CAM_TILT_NEUTRAL
  float tilt_diff = cam_tilt_c - RadOfDeg(CAM_TILT_NEUTRAL);
  if (tilt_diff > 0)
    cam_tilt = MAX_PPRZ * (tilt_diff / (RadOfDeg(CAM_TILT_MAX - CAM_TILT_NEUTRAL)));
  else
    cam_tilt = MIN_PPRZ * (tilt_diff / (RadOfDeg(CAM_TILT_MIN - CAM_TILT_NEUTRAL)));
#endif

  cam_pan = TRIM_PPRZ(cam_pan);
  cam_tilt = TRIM_PPRZ(cam_tilt);

  cam_phi_c = cam_pan_c;
  cam_theta_c = cam_tilt_c;

#ifdef COMMAND_CAM_PAN
  ap_state->commands[COMMAND_CAM_PAN] = cam_pan;
#endif
#ifdef COMMAND_CAM_TILT
  ap_state->commands[COMMAND_CAM_TILT] = cam_tilt;
#endif
}

/** Computes the right angles from target_x, target_y, target_alt */
void cam_target( void ) {
#ifdef TEST_CAM
  vPoint(test_cam_estimator_x, test_cam_estimator_y, test_cam_estimator_z,
     test_cam_estimator_phi, test_cam_estimator_theta, test_cam_estimator_hspeed_dir,
     cam_target_x, cam_target_y, cam_target_alt,
     &cam_pan_c, &cam_tilt_c);
#else
  vPoint(estimator_x, estimator_y, estimator_z,
     estimator_phi, estimator_theta, estimator_hspeed_dir,
     cam_target_x, cam_target_y, cam_target_alt,
     &cam_pan_c, &cam_tilt_c);
#endif
  cam_angles();
}

/** Point straight down */
void cam_nadir( void ) {
#ifdef TEST_CAM
  cam_target_x = test_cam_estimator_x;
  cam_target_y = test_cam_estimator_y;
#else
  cam_target_x = estimator_x;
  cam_target_y = estimator_y;
#endif
  cam_target_alt = 0;
  cam_target();
}


void cam_waypoint_target( void ) {
  if (cam_target_wp < nb_waypoint) {
    cam_target_x = waypoints[cam_target_wp].x;
    cam_target_y = waypoints[cam_target_wp].y;
  }
  cam_target_alt = ground_alt;
  cam_target();
}

void cam_ac_target( void ) {
#ifdef TRAFFIC_INFO
  struct ac_info_ * ac = get_ac_info(cam_target_ac);
  cam_target_x = ac->east;
  cam_target_y = ac->north;
  cam_target_alt = ac->alt;
  cam_target();
#endif // TRAFFIC_INFO
}

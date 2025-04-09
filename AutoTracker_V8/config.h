#ifndef CONFIG_H
#define CONFIG_H

/*

   1/3 sensor       : 4.8mm x 3.6mm | 6mm
   focal length     : 56mm

   Angle of view    : 4.908 x 3.682 | 6.133

   Field of View    :  @500 m | 42.86 x 32.14 | 53.57
                    : @3500 m |   300 x 225   | 375

   MinSpeed in pixels: 2km/h @3500m = 0.0231481481481481 meters per frame

*/

#define _H_AOV          (1.763169f)
#define _V_AOV          (1.042001f)

#define CP_SERIALPATH "/dev/ttyUSB0"
//#define CP_SERIALPATH "/dev/ttyACM0"

#define GIMBAL_PORT 30544
#define GIMBAL_SERIAL ""
#define MAX_MULTI_TARGETS 2
#define DESK_PANEL_PORT 44403
//#define DESK_PANEL_ADDRESS "192.168.20.204"
#define DESK_PANEL_ADDRESS "127.0.0.1"

#define X_TRACKING_MAX  192
#define X_TRACKING_MAXI 48
#define X_TRACKING_KP 4.0f
#define X_TRACKING_KI 0.5f
#define X_TRACKING_KD 0.01

#define Y_TRACKING_MAX  192
#define Y_TRACKING_MAXI 48
#define Y_TRACKING_KP 4.0f
#define Y_TRACKING_KI 0.5f
#define Y_TRACKING_KD 0.01

#define IMAGE_PORT 12345

//#define SERVER_IP "192.168.20.204"
#define SERVER_IP "127.0.0.1"
#define IMAGE_SOCKET 40305
#define CONTROLLER_SOCKET 30405

#define RC_IMAGE_PORT 40435
#define RC_DATA_PORT 43454
#define RC_MAX_CONN 4

#define TELE_DATA_PORT 54777

#define UPDATE_LOOP_TIME (1000/100)
#define MAX_TARGET_WIDTH 120
#define MAX_TARGET_HEIGHT 80

#define DISTANCE_TO_FOV 0.085713333f

enum TRACKER_TYPE { CSRT = 0, KCF, GOTURN, DaSiamRPN, YOLO, YOLO_CLIENT, OPFeature, TEMPLATE_MATCHER, OPFlow, State_EST,  FUSION, tt_count };
//enum TRACKER_TYPE { CSRT = 0, KCF, GOTURN, DaSiamRPN, YOLO, YOLO_CLIENT, OPFeature, TEMPLATE_MATCHER, OPFlow, State_EST,  FUSION, tt_count };

#endif // CONFIG_H

////////////////////////////////////////////////////////////////////////////////
// @filename: muJoeGenericProfile.h
// @author: Joseph Corteo Jr.
////////////////////////////////////////////////////////////////////////////////

#ifndef MUJOEGENERICPROFILE_H
#define MUJOEGENERICPROFILE_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// DEFINE
////////////////////////////////////////////////////////////////////////////////

// Profile Parameter IDs
#define MUJOEGENERICPROFILE_COMMAND             0  // RW uint8 - Profile Command Characteristic value 
#define MUJOEGENERICPROFILE_RESPONSE            1  // R uint8 - Profile Response Characteristic value 
#define MUJOEGENERICPROFILE_MAILBOX             2  // RW uint8 - Profile Mailbox Characteristic value       
#define MUJOEGENERICPROFILE_DEVICEINFO          3  // R uint8 - Profile Device Info Characteristic value

// muJoe Generic Profile Services bit fields
#define MUJOEGENERICPROFILE_SERVICE               0x00000001

// muJoe Generic Profile Service UUID
#define MUJOEGENERICPROFILE_SERV_UUID             0xFFF0

// Characteristic UUIDs
#define MUJOEGENERICPROFILE_COMMAND_UUID          0xFFF1
#define MUJOEGENERICPROFILE_RESPONSE_UUID         0xFFF2
#define MUJOEGENERICPROFILE_MAILBOX_UUID          0xFFF3
#define MUJOEGENERICPROFILE_DEVICEINFO_UUID       0xFFF4

// Length of Command Characteristic in bytes
#define MUJOEGENERICPROFILE_CMD_LEN               2

// Length of Reponse Characteristic in bytes
#define MUJOEGENERICPROFILE_RSP_LEN               2

// Length of Mailbox Characteristic in bytes
#define MUJOEGENERICPROFILE_MBOX_LEN              20

// Length of Device Info Characteristic in bytes
#define MUJOEGENERICPROFILE_DEVINFO_LEN           4


////////////////////////////////////////////////////////////////////////////////
// Profile Callbacks
////////////////////////////////////////////////////////////////////////////////

// Callback when a characteristic value has changed
typedef void (*muJoeGenProfileChange_t)( uint8 paramID );

typedef struct
{
  muJoeGenProfileChange_t        pfnSimpleProfileChange;  // Called when characteristic value changes

} muJoeGenProfileCBs_t;

////////////////////////////////////////////////////////////////////////////////
// PROTOS
////////////////////////////////////////////////////////////////////////////////

/*
 * MuJoeGenericProfile_AddService- Initializes the muJoe Generic GATT Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 */

extern bStatus_t MuJoeGenericProfile_AddService( void );


/*
 * muJoeGenProfile_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t muJoeGenProfile_RegisterAppCBs( muJoeGenProfileCBs_t *appCallbacks );


#endif
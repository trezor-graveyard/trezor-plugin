/* 
 * File:   messages.h
 * Author: arembish
 *
 * Created on April 6, 2013, 4:21 PM
 */

#ifndef MESSAGES_H
#define	MESSAGES_H

/*
 *        0: proto.Initialize,
       1: proto.Ping,
       2: proto.Success,
       3: proto.Failure,
       4: proto.GetUUID,
       5: proto.UUID,
       6: proto.OtpRequest,
       7: proto.OtpAck,
       8: proto.OtpCancel,
       9: proto.GetEntropy,
       10: proto.Entropy,
       11: proto.GetMasterPublicKey,
       12: proto.MasterPublicKey,
       13: proto.LoadDevice,
       14: proto.ResetDevice,
       15: proto.SignTx,
#       16: proto.SignedTx,
       17: proto.Features,
       18: proto.PinRequest,
       19: proto.PinAck,
       20: proto.PinCancel,
       21: proto.TxRequest,
       #22: proto.OutputRequest,
       23: proto.TxInput,
       24: proto.TxOutput,
       25: proto.SetMaxFeeKb,
       26: proto.ButtonRequest,
       27: proto.ButtonAck,
       28: proto.ButtonCancel,
       29: proto.GetAddress,
       30: proto.Address,
 */

#define TYPE_Initialize 0
#define TYPE_Ping 1
#define TYPE_Success 2
#define TYPE_Failure 3
#define TYPE_GetUUID 4
#define TYPE_UUID 5
#define TYPE_GetEntropy 9
#define TYPE_Entropy 10
#define TYPE_GetMasterPublicKey 11
#define TYPE_MasterPublicKey 12
#define TYPE_Features 17
#define TYPE_ButtonRequest 26
#define TYPE_ButtonAck 27
#define TYPE_ButtonCancel 28
#define TYPE_GetAddress 29
#define TYPE_Address 30

#define MESSAGE_TYPE(X) TYPE_ ## X

#endif	/* MESSAGES_H */


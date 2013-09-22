/*
 * File:   messages.h
 * Author: arembish
 *
 * Created on April 6, 2013, 4:21 PM
 */

#ifndef MESSAGES_H
#define	MESSAGES_H

/*
 * From trezor-emu/trezor/mapping.py,
 * commit 3fef5041362fbbe56963964a909c95b0e7e45e39

   0: proto.Initialize,
   1: proto.Ping,
   2: proto.Success,
   3: proto.Failure,
#  4: proto.GetUID,
#  5: proto.UUID,
   9: proto.GetEntropy,
  10: proto.Entropy,
  11: proto.GetMasterPublicKey,
  12: proto.MasterPublicKey,
  13: proto.LoadDevice,
  14: proto.ResetDevice,
  15: proto.SignTx,
  16: proto.SimpleSignTx,
  17: proto.Features,
  18: proto.PinMatrixRequest,
  19: proto.PinMatrixAck,
  20: proto.PinMatrixCancel,
  21: proto.TxRequest,
# 22: proto.OutputRequest,
  23: proto.TxInput,
  24: proto.TxOutput,
  25: proto.ApplySettings,
  26: proto.ButtonRequest,
  27: proto.ButtonAck,
  28: proto.ButtonCancel,
  29: proto.GetAddress,
  30: proto.Address,
  31: proto.SettingsType,
  32: proto.XprvType,
  33: proto.CoinType,
# ... debug messages intentionaly ignored
*/

#define TYPE_Initialize 0
#define TYPE_Ping 1
#define TYPE_Success 2
#define TYPE_Failure 3
#define TYPE_GetEntropy 9
#define TYPE_Entropy 10
#define TYPE_GetMasterPublicKey 11
#define TYPE_MasterPublicKey 12
#define TYPE_LoadDevice 13
#define TYPE_ResetDevice 14
#define TYPE_SignTx 15
#define TYPE_SimpleSignTx 16
#define TYPE_Features 17
#define TYPE_PinMatrixRequest 18
#define TYPE_PinMatrixAck 19
#define TYPE_PinMatrixCancel 20
#define TYPE_TxRequest 21
#define TYPE_TxInput 23
#define TYPE_TxOutput 24
#define TYPE_ApplySettings 25
#define TYPE_ButtonRequest 26
#define TYPE_ButtonAck 27
#define TYPE_ButtonCancel 28
#define TYPE_GetAddress 29
#define TYPE_Address 30
#define TYPE_SettingsType 31
#define TYPE_XprvType 32
#define TYPE_CoinType 33

#define MESSAGE_TYPE(X) TYPE_ ## X

#endif	/* MESSAGES_H */

/* This file is automatically generated by aarch64-gen.  Do not edit!  */
/* Copyright (C) 2012-2019 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "aarch64-asm.h"


const aarch64_opcode *
aarch64_find_real_opcode (const aarch64_opcode *opcode)
{
  /* Use the index as the key to locate the real opcode.  */
  int key = opcode - aarch64_opcode_table;
  int value;
  switch (key)
    {
    case 3:	/* ngc */
    case 2:	/* sbc */
      value = 2;	/* --> sbc.  */
      break;
    case 5:	/* ngcs */
    case 4:	/* sbcs */
      value = 4;	/* --> sbcs.  */
      break;
    case 8:	/* cmn */
    case 7:	/* adds */
      value = 7;	/* --> adds.  */
      break;
    case 11:	/* cmp */
    case 10:	/* subs */
      value = 10;	/* --> subs.  */
      break;
    case 13:	/* mov */
    case 12:	/* add */
      value = 12;	/* --> add.  */
      break;
    case 15:	/* cmn */
    case 14:	/* adds */
      value = 14;	/* --> adds.  */
      break;
    case 18:	/* cmp */
    case 17:	/* subs */
      value = 17;	/* --> subs.  */
      break;
    case 23:	/* cmn */
    case 22:	/* adds */
      value = 22;	/* --> adds.  */
      break;
    case 25:	/* neg */
    case 24:	/* sub */
      value = 24;	/* --> sub.  */
      break;
    case 27:	/* cmp */
    case 28:	/* negs */
    case 26:	/* subs */
      value = 26;	/* --> subs.  */
      break;
    case 153:	/* mov */
    case 152:	/* umov */
      value = 152;	/* --> umov.  */
      break;
    case 155:	/* mov */
    case 154:	/* ins */
      value = 154;	/* --> ins.  */
      break;
    case 157:	/* mov */
    case 156:	/* ins */
      value = 156;	/* --> ins.  */
      break;
    case 243:	/* mvn */
    case 242:	/* not */
      value = 242;	/* --> not.  */
      break;
    case 318:	/* mov */
    case 317:	/* orr */
      value = 317;	/* --> orr.  */
      break;
    case 389:	/* sxtl */
    case 388:	/* sshll */
      value = 388;	/* --> sshll.  */
      break;
    case 391:	/* sxtl2 */
    case 390:	/* sshll2 */
      value = 390;	/* --> sshll2.  */
      break;
    case 413:	/* uxtl */
    case 412:	/* ushll */
      value = 412;	/* --> ushll.  */
      break;
    case 415:	/* uxtl2 */
    case 414:	/* ushll2 */
      value = 414;	/* --> ushll2.  */
      break;
    case 536:	/* mov */
    case 535:	/* dup */
      value = 535;	/* --> dup.  */
      break;
    case 623:	/* sxtw */
    case 622:	/* sxth */
    case 621:	/* sxtb */
    case 624:	/* asr */
    case 620:	/* sbfx */
    case 619:	/* sbfiz */
    case 618:	/* sbfm */
      value = 618;	/* --> sbfm.  */
      break;
    case 627:	/* bfc */
    case 628:	/* bfxil */
    case 626:	/* bfi */
    case 625:	/* bfm */
      value = 625;	/* --> bfm.  */
      break;
    case 633:	/* uxth */
    case 632:	/* uxtb */
    case 635:	/* lsr */
    case 634:	/* lsl */
    case 631:	/* ubfx */
    case 630:	/* ubfiz */
    case 629:	/* ubfm */
      value = 629;	/* --> ubfm.  */
      break;
    case 665:	/* cset */
    case 664:	/* cinc */
    case 663:	/* csinc */
      value = 663;	/* --> csinc.  */
      break;
    case 668:	/* csetm */
    case 667:	/* cinv */
    case 666:	/* csinv */
      value = 666;	/* --> csinv.  */
      break;
    case 670:	/* cneg */
    case 669:	/* csneg */
      value = 669;	/* --> csneg.  */
      break;
    case 688:	/* rev */
    case 689:	/* rev64 */
      value = 688;	/* --> rev.  */
      break;
    case 714:	/* lsl */
    case 713:	/* lslv */
      value = 713;	/* --> lslv.  */
      break;
    case 716:	/* lsr */
    case 715:	/* lsrv */
      value = 715;	/* --> lsrv.  */
      break;
    case 718:	/* asr */
    case 717:	/* asrv */
      value = 717;	/* --> asrv.  */
      break;
    case 720:	/* ror */
    case 719:	/* rorv */
      value = 719;	/* --> rorv.  */
      break;
    case 723:	/* cmpp */
    case 722:	/* subps */
      value = 722;	/* --> subps.  */
      break;
    case 736:	/* mul */
    case 735:	/* madd */
      value = 735;	/* --> madd.  */
      break;
    case 738:	/* mneg */
    case 737:	/* msub */
      value = 737;	/* --> msub.  */
      break;
    case 740:	/* smull */
    case 739:	/* smaddl */
      value = 739;	/* --> smaddl.  */
      break;
    case 742:	/* smnegl */
    case 741:	/* smsubl */
      value = 741;	/* --> smsubl.  */
      break;
    case 745:	/* umull */
    case 744:	/* umaddl */
      value = 744;	/* --> umaddl.  */
      break;
    case 747:	/* umnegl */
    case 746:	/* umsubl */
      value = 746;	/* --> umsubl.  */
      break;
    case 758:	/* ror */
    case 757:	/* extr */
      value = 757;	/* --> extr.  */
      break;
    case 991:	/* bic */
    case 990:	/* and */
      value = 990;	/* --> and.  */
      break;
    case 993:	/* mov */
    case 992:	/* orr */
      value = 992;	/* --> orr.  */
      break;
    case 996:	/* tst */
    case 995:	/* ands */
      value = 995;	/* --> ands.  */
      break;
    case 1001:	/* uxtw */
    case 1000:	/* mov */
    case 999:	/* orr */
      value = 999;	/* --> orr.  */
      break;
    case 1003:	/* mvn */
    case 1002:	/* orn */
      value = 1002;	/* --> orn.  */
      break;
    case 1007:	/* tst */
    case 1006:	/* ands */
      value = 1006;	/* --> ands.  */
      break;
    case 1133:	/* staddb */
    case 1037:	/* ldaddb */
      value = 1037;	/* --> ldaddb.  */
      break;
    case 1134:	/* staddh */
    case 1038:	/* ldaddh */
      value = 1038;	/* --> ldaddh.  */
      break;
    case 1135:	/* stadd */
    case 1039:	/* ldadd */
      value = 1039;	/* --> ldadd.  */
      break;
    case 1136:	/* staddlb */
    case 1041:	/* ldaddlb */
      value = 1041;	/* --> ldaddlb.  */
      break;
    case 1137:	/* staddlh */
    case 1044:	/* ldaddlh */
      value = 1044;	/* --> ldaddlh.  */
      break;
    case 1138:	/* staddl */
    case 1047:	/* ldaddl */
      value = 1047;	/* --> ldaddl.  */
      break;
    case 1139:	/* stclrb */
    case 1049:	/* ldclrb */
      value = 1049;	/* --> ldclrb.  */
      break;
    case 1140:	/* stclrh */
    case 1050:	/* ldclrh */
      value = 1050;	/* --> ldclrh.  */
      break;
    case 1141:	/* stclr */
    case 1051:	/* ldclr */
      value = 1051;	/* --> ldclr.  */
      break;
    case 1142:	/* stclrlb */
    case 1053:	/* ldclrlb */
      value = 1053;	/* --> ldclrlb.  */
      break;
    case 1143:	/* stclrlh */
    case 1056:	/* ldclrlh */
      value = 1056;	/* --> ldclrlh.  */
      break;
    case 1144:	/* stclrl */
    case 1059:	/* ldclrl */
      value = 1059;	/* --> ldclrl.  */
      break;
    case 1145:	/* steorb */
    case 1061:	/* ldeorb */
      value = 1061;	/* --> ldeorb.  */
      break;
    case 1146:	/* steorh */
    case 1062:	/* ldeorh */
      value = 1062;	/* --> ldeorh.  */
      break;
    case 1147:	/* steor */
    case 1063:	/* ldeor */
      value = 1063;	/* --> ldeor.  */
      break;
    case 1148:	/* steorlb */
    case 1065:	/* ldeorlb */
      value = 1065;	/* --> ldeorlb.  */
      break;
    case 1149:	/* steorlh */
    case 1068:	/* ldeorlh */
      value = 1068;	/* --> ldeorlh.  */
      break;
    case 1150:	/* steorl */
    case 1071:	/* ldeorl */
      value = 1071;	/* --> ldeorl.  */
      break;
    case 1151:	/* stsetb */
    case 1073:	/* ldsetb */
      value = 1073;	/* --> ldsetb.  */
      break;
    case 1152:	/* stseth */
    case 1074:	/* ldseth */
      value = 1074;	/* --> ldseth.  */
      break;
    case 1153:	/* stset */
    case 1075:	/* ldset */
      value = 1075;	/* --> ldset.  */
      break;
    case 1154:	/* stsetlb */
    case 1077:	/* ldsetlb */
      value = 1077;	/* --> ldsetlb.  */
      break;
    case 1155:	/* stsetlh */
    case 1080:	/* ldsetlh */
      value = 1080;	/* --> ldsetlh.  */
      break;
    case 1156:	/* stsetl */
    case 1083:	/* ldsetl */
      value = 1083;	/* --> ldsetl.  */
      break;
    case 1157:	/* stsmaxb */
    case 1085:	/* ldsmaxb */
      value = 1085;	/* --> ldsmaxb.  */
      break;
    case 1158:	/* stsmaxh */
    case 1086:	/* ldsmaxh */
      value = 1086;	/* --> ldsmaxh.  */
      break;
    case 1159:	/* stsmax */
    case 1087:	/* ldsmax */
      value = 1087;	/* --> ldsmax.  */
      break;
    case 1160:	/* stsmaxlb */
    case 1089:	/* ldsmaxlb */
      value = 1089;	/* --> ldsmaxlb.  */
      break;
    case 1161:	/* stsmaxlh */
    case 1092:	/* ldsmaxlh */
      value = 1092;	/* --> ldsmaxlh.  */
      break;
    case 1162:	/* stsmaxl */
    case 1095:	/* ldsmaxl */
      value = 1095;	/* --> ldsmaxl.  */
      break;
    case 1163:	/* stsminb */
    case 1097:	/* ldsminb */
      value = 1097;	/* --> ldsminb.  */
      break;
    case 1164:	/* stsminh */
    case 1098:	/* ldsminh */
      value = 1098;	/* --> ldsminh.  */
      break;
    case 1165:	/* stsmin */
    case 1099:	/* ldsmin */
      value = 1099;	/* --> ldsmin.  */
      break;
    case 1166:	/* stsminlb */
    case 1101:	/* ldsminlb */
      value = 1101;	/* --> ldsminlb.  */
      break;
    case 1167:	/* stsminlh */
    case 1104:	/* ldsminlh */
      value = 1104;	/* --> ldsminlh.  */
      break;
    case 1168:	/* stsminl */
    case 1107:	/* ldsminl */
      value = 1107;	/* --> ldsminl.  */
      break;
    case 1169:	/* stumaxb */
    case 1109:	/* ldumaxb */
      value = 1109;	/* --> ldumaxb.  */
      break;
    case 1170:	/* stumaxh */
    case 1110:	/* ldumaxh */
      value = 1110;	/* --> ldumaxh.  */
      break;
    case 1171:	/* stumax */
    case 1111:	/* ldumax */
      value = 1111;	/* --> ldumax.  */
      break;
    case 1172:	/* stumaxlb */
    case 1113:	/* ldumaxlb */
      value = 1113;	/* --> ldumaxlb.  */
      break;
    case 1173:	/* stumaxlh */
    case 1116:	/* ldumaxlh */
      value = 1116;	/* --> ldumaxlh.  */
      break;
    case 1174:	/* stumaxl */
    case 1119:	/* ldumaxl */
      value = 1119;	/* --> ldumaxl.  */
      break;
    case 1175:	/* stuminb */
    case 1121:	/* lduminb */
      value = 1121;	/* --> lduminb.  */
      break;
    case 1176:	/* stuminh */
    case 1122:	/* lduminh */
      value = 1122;	/* --> lduminh.  */
      break;
    case 1177:	/* stumin */
    case 1123:	/* ldumin */
      value = 1123;	/* --> ldumin.  */
      break;
    case 1178:	/* stuminlb */
    case 1125:	/* lduminlb */
      value = 1125;	/* --> lduminlb.  */
      break;
    case 1179:	/* stuminlh */
    case 1128:	/* lduminlh */
      value = 1128;	/* --> lduminlh.  */
      break;
    case 1180:	/* stuminl */
    case 1131:	/* lduminl */
      value = 1131;	/* --> lduminl.  */
      break;
    case 1182:	/* mov */
    case 1181:	/* movn */
      value = 1181;	/* --> movn.  */
      break;
    case 1184:	/* mov */
    case 1183:	/* movz */
      value = 1183;	/* --> movz.  */
      break;
    case 1234:	/* autibsp */
    case 1233:	/* autibz */
    case 1232:	/* autiasp */
    case 1231:	/* autiaz */
    case 1230:	/* pacibsp */
    case 1229:	/* pacibz */
    case 1228:	/* paciasp */
    case 1227:	/* paciaz */
    case 1208:	/* psb */
    case 1207:	/* esb */
    case 1206:	/* autib1716 */
    case 1205:	/* autia1716 */
    case 1204:	/* pacib1716 */
    case 1203:	/* pacia1716 */
    case 1202:	/* xpaclri */
    case 1201:	/* sevl */
    case 1200:	/* sev */
    case 1199:	/* wfi */
    case 1198:	/* wfe */
    case 1197:	/* yield */
    case 1196:	/* bti */
    case 1195:	/* csdb */
    case 1194:	/* nop */
    case 1193:	/* hint */
      value = 1193;	/* --> hint.  */
      break;
    case 1212:	/* pssbb */
    case 1211:	/* ssbb */
    case 1210:	/* dsb */
      value = 1210;	/* --> dsb.  */
      break;
    case 1223:	/* cpp */
    case 1222:	/* dvp */
    case 1221:	/* cfp */
    case 1220:	/* tlbi */
    case 1219:	/* ic */
    case 1218:	/* dc */
    case 1217:	/* at */
    case 1216:	/* sys */
      value = 1216;	/* --> sys.  */
      break;
    case 2032:	/* bic */
    case 1282:	/* and */
      value = 1282;	/* --> and.  */
      break;
    case 1265:	/* mov */
    case 1284:	/* and */
      value = 1284;	/* --> and.  */
      break;
    case 1269:	/* movs */
    case 1285:	/* ands */
      value = 1285;	/* --> ands.  */
      break;
    case 2033:	/* cmple */
    case 1320:	/* cmpge */
      value = 1320;	/* --> cmpge.  */
      break;
    case 2036:	/* cmplt */
    case 1323:	/* cmpgt */
      value = 1323;	/* --> cmpgt.  */
      break;
    case 2034:	/* cmplo */
    case 1325:	/* cmphi */
      value = 1325;	/* --> cmphi.  */
      break;
    case 2035:	/* cmpls */
    case 1328:	/* cmphs */
      value = 1328;	/* --> cmphs.  */
      break;
    case 1262:	/* mov */
    case 1350:	/* cpy */
      value = 1350;	/* --> cpy.  */
      break;
    case 1264:	/* mov */
    case 1351:	/* cpy */
      value = 1351;	/* --> cpy.  */
      break;
    case 2043:	/* fmov */
    case 1267:	/* mov */
    case 1352:	/* cpy */
      value = 1352;	/* --> cpy.  */
      break;
    case 1257:	/* mov */
    case 1364:	/* dup */
      value = 1364;	/* --> dup.  */
      break;
    case 1259:	/* mov */
    case 1256:	/* mov */
    case 1365:	/* dup */
      value = 1365;	/* --> dup.  */
      break;
    case 2042:	/* fmov */
    case 1261:	/* mov */
    case 1366:	/* dup */
      value = 1366;	/* --> dup.  */
      break;
    case 1260:	/* mov */
    case 1367:	/* dupm */
      value = 1367;	/* --> dupm.  */
      break;
    case 2037:	/* eon */
    case 1369:	/* eor */
      value = 1369;	/* --> eor.  */
      break;
    case 1270:	/* not */
    case 1371:	/* eor */
      value = 1371;	/* --> eor.  */
      break;
    case 1271:	/* nots */
    case 1372:	/* eors */
      value = 1372;	/* --> eors.  */
      break;
    case 2038:	/* facle */
    case 1377:	/* facge */
      value = 1377;	/* --> facge.  */
      break;
    case 2039:	/* faclt */
    case 1378:	/* facgt */
      value = 1378;	/* --> facgt.  */
      break;
    case 2040:	/* fcmle */
    case 1391:	/* fcmge */
      value = 1391;	/* --> fcmge.  */
      break;
    case 2041:	/* fcmlt */
    case 1393:	/* fcmgt */
      value = 1393;	/* --> fcmgt.  */
      break;
    case 1254:	/* fmov */
    case 1399:	/* fcpy */
      value = 1399;	/* --> fcpy.  */
      break;
    case 1253:	/* fmov */
    case 1422:	/* fdup */
      value = 1422;	/* --> fdup.  */
      break;
    case 1255:	/* mov */
    case 1753:	/* orr */
      value = 1753;	/* --> orr.  */
      break;
    case 2044:	/* orn */
    case 1754:	/* orr */
      value = 1754;	/* --> orr.  */
      break;
    case 1258:	/* mov */
    case 1756:	/* orr */
      value = 1756;	/* --> orr.  */
      break;
    case 1268:	/* movs */
    case 1757:	/* orrs */
      value = 1757;	/* --> orrs.  */
      break;
    case 1263:	/* mov */
    case 1819:	/* sel */
      value = 1819;	/* --> sel.  */
      break;
    case 1266:	/* mov */
    case 1820:	/* sel */
      value = 1820;	/* --> sel.  */
      break;
    default: return NULL;
    }

  return aarch64_opcode_table + value;
}

bfd_boolean
aarch64_insert_operand (const aarch64_operand *self,
			   const aarch64_opnd_info *info,
			   aarch64_insn *code, const aarch64_inst *inst,
			   aarch64_operand_error *errors)
{
  /* Use the index as the key.  */
  int key = self - aarch64_operands;
  switch (key)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 16:
    case 17:
    case 18:
    case 19:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 162:
    case 163:
    case 164:
    case 165:
    case 166:
    case 167:
    case 168:
    case 169:
    case 170:
    case 171:
    case 186:
    case 187:
    case 188:
    case 189:
    case 190:
    case 191:
    case 192:
    case 193:
    case 194:
    case 200:
    case 203:
      return aarch64_ins_regno (self, info, code, inst, errors);
    case 14:
      return aarch64_ins_reg_extended (self, info, code, inst, errors);
    case 15:
      return aarch64_ins_reg_shifted (self, info, code, inst, errors);
    case 20:
      return aarch64_ins_ft (self, info, code, inst, errors);
    case 31:
    case 32:
    case 33:
    case 34:
    case 206:
      return aarch64_ins_reglane (self, info, code, inst, errors);
    case 35:
      return aarch64_ins_reglist (self, info, code, inst, errors);
    case 36:
      return aarch64_ins_ldst_reglist (self, info, code, inst, errors);
    case 37:
      return aarch64_ins_ldst_reglist_r (self, info, code, inst, errors);
    case 38:
      return aarch64_ins_ldst_elemlist (self, info, code, inst, errors);
    case 39:
    case 40:
    case 41:
    case 42:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 65:
    case 66:
    case 67:
    case 79:
    case 80:
    case 81:
    case 82:
    case 159:
    case 161:
    case 178:
    case 179:
    case 180:
    case 181:
    case 182:
    case 183:
    case 184:
    case 185:
    case 205:
      return aarch64_ins_imm (self, info, code, inst, errors);
    case 43:
    case 44:
      return aarch64_ins_advsimd_imm_shift (self, info, code, inst, errors);
    case 45:
    case 46:
    case 47:
      return aarch64_ins_advsimd_imm_modified (self, info, code, inst, errors);
    case 51:
    case 149:
      return aarch64_ins_fpimm (self, info, code, inst, errors);
    case 68:
    case 157:
      return aarch64_ins_limm (self, info, code, inst, errors);
    case 69:
      return aarch64_ins_aimm (self, info, code, inst, errors);
    case 70:
      return aarch64_ins_imm_half (self, info, code, inst, errors);
    case 71:
      return aarch64_ins_fbits (self, info, code, inst, errors);
    case 73:
    case 74:
    case 154:
      return aarch64_ins_imm_rotate2 (self, info, code, inst, errors);
    case 75:
    case 153:
    case 155:
      return aarch64_ins_imm_rotate1 (self, info, code, inst, errors);
    case 76:
    case 77:
      return aarch64_ins_cond (self, info, code, inst, errors);
    case 83:
    case 92:
      return aarch64_ins_addr_simple (self, info, code, inst, errors);
    case 84:
      return aarch64_ins_addr_regoff (self, info, code, inst, errors);
    case 85:
    case 86:
    case 87:
    case 89:
    case 91:
      return aarch64_ins_addr_simm (self, info, code, inst, errors);
    case 88:
      return aarch64_ins_addr_simm10 (self, info, code, inst, errors);
    case 90:
      return aarch64_ins_addr_uimm12 (self, info, code, inst, errors);
    case 93:
      return aarch64_ins_addr_offset (self, info, code, inst, errors);
    case 94:
      return aarch64_ins_simd_addr_post (self, info, code, inst, errors);
    case 95:
      return aarch64_ins_sysreg (self, info, code, inst, errors);
    case 96:
      return aarch64_ins_pstatefield (self, info, code, inst, errors);
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
      return aarch64_ins_sysins_op (self, info, code, inst, errors);
    case 102:
    case 103:
      return aarch64_ins_barrier (self, info, code, inst, errors);
    case 104:
      return aarch64_ins_prfop (self, info, code, inst, errors);
    case 105:
    case 106:
      return aarch64_ins_hint (self, info, code, inst, errors);
    case 107:
      return aarch64_ins_sve_addr_ri_s4 (self, info, code, inst, errors);
    case 108:
    case 109:
    case 110:
    case 111:
      return aarch64_ins_sve_addr_ri_s4xvl (self, info, code, inst, errors);
    case 112:
      return aarch64_ins_sve_addr_ri_s6xvl (self, info, code, inst, errors);
    case 113:
      return aarch64_ins_sve_addr_ri_s9xvl (self, info, code, inst, errors);
    case 114:
    case 115:
    case 116:
    case 117:
      return aarch64_ins_sve_addr_ri_u6 (self, info, code, inst, errors);
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
    case 123:
    case 124:
    case 125:
    case 126:
    case 127:
    case 128:
    case 129:
    case 130:
    case 131:
      return aarch64_ins_sve_addr_rr_lsl (self, info, code, inst, errors);
    case 132:
    case 133:
    case 134:
    case 135:
    case 136:
    case 137:
    case 138:
    case 139:
      return aarch64_ins_sve_addr_rz_xtw (self, info, code, inst, errors);
    case 140:
    case 141:
    case 142:
    case 143:
      return aarch64_ins_sve_addr_zi_u5 (self, info, code, inst, errors);
    case 144:
      return aarch64_ins_sve_addr_zz_lsl (self, info, code, inst, errors);
    case 145:
      return aarch64_ins_sve_addr_zz_sxtw (self, info, code, inst, errors);
    case 146:
      return aarch64_ins_sve_addr_zz_uxtw (self, info, code, inst, errors);
    case 147:
      return aarch64_ins_sve_aimm (self, info, code, inst, errors);
    case 148:
      return aarch64_ins_sve_asimm (self, info, code, inst, errors);
    case 150:
      return aarch64_ins_sve_float_half_one (self, info, code, inst, errors);
    case 151:
      return aarch64_ins_sve_float_half_two (self, info, code, inst, errors);
    case 152:
      return aarch64_ins_sve_float_zero_one (self, info, code, inst, errors);
    case 156:
      return aarch64_ins_inv_limm (self, info, code, inst, errors);
    case 158:
      return aarch64_ins_sve_limm_mov (self, info, code, inst, errors);
    case 160:
      return aarch64_ins_sve_scale (self, info, code, inst, errors);
    case 172:
    case 173:
    case 174:
      return aarch64_ins_sve_shlimm (self, info, code, inst, errors);
    case 175:
    case 176:
    case 177:
      return aarch64_ins_sve_shrimm (self, info, code, inst, errors);
    case 195:
    case 196:
    case 197:
    case 198:
    case 199:
      return aarch64_ins_sve_quad_index (self, info, code, inst, errors);
    case 201:
      return aarch64_ins_sve_index (self, info, code, inst, errors);
    case 202:
    case 204:
      return aarch64_ins_sve_reglist (self, info, code, inst, errors);
    default: assert (0); abort ();
    }
}
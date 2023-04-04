/*
 * checksum.h
 *
 *  Created on: Feb 16, 2021
 *      Author: chenyu
 */

#ifndef INC_CHECKSUM_H_
#define INC_CHECKSUM_H_

unsigned short gf_crc_rawdata(unsigned char *param_1,unsigned short param_2);
long long gf_checksum_rawdata(unsigned char *a1, int a2);

#endif /* INC_CHECKSUM_H_ */

/*
 * macro.h
 *
 *  Created on: 2010/06/06
 *      Author: takka
 */

#ifndef MACRO_H_
#define MACRO_H_

#define bin2int(var, addr)                        \
  {                                                 \
    *(((char *)(var)) + 0) = *(((char *)(addr)) + 0);  \
    *(((char *)(var)) + 1) = *(((char *)(addr)) + 1);  \
    *(((char *)(var)) + 2) = *(((char *)(addr)) + 2);  \
    *(((char *)(var)) + 3) = *(((char *)(addr)) + 3);  \
  }                                                 \

#endif /* MACRO_H_ */

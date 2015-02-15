/*
 * nor.h
 *
 *  Created on: 15.02.2015
 *      Author: ALev
 */

#ifndef EMB_NOR_H
#define EMB_NOR_H

bool nor_write(unsigned int iblock, const void* buf, unsigned int blocks);
bool nor_read(unsigned int iblock, void* buf, unsigned int blocks);

#endif


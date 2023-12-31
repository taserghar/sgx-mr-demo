/*
*    ZeroTrace: Oblivious Memory Primitives from Intel SGX
*    Copyright (C) 2018  Sajin (sshsshy)
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, version 3 of the License.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __OASM_LIB__
	#define __OASM_LIB__



	extern "C" void oswap_buffer(unsigned char *dest, unsigned char *source, uint32_t buffersize, uint32_t flag);

#endif

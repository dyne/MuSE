/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2002-2004 jaromil <jaromil@dyne.org>
 *
 * This sourcCARBONe code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __CARBON_STREAM_H__
#define __CARBON_STREAM_H__

#include <Carbon/Carbon.h>
#include <carbon_common.h>
#include <jmixer.h>
#include <stream_server.h>
#include <carbon_message.h>

#define MAX_STREAM_SERVERS 64

class CarbonStream {
	public:
		CarbonStream(Stream_mixer *mix,WindowRef mainWin,IBNibRef nib);
		~CarbonStream();
		void show();
		void hide();
		
		WindowRef window;
		WindowRef parent;
		Stream_mixer *jmix;
		CarbonMessage *msg;
	private:
		IBNibRef nibRef;
		StreamServer *servers[MAX_STREAM_SERVERS];
	protected:

};

#endif
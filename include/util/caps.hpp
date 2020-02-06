/*
 * Copyright (c) 2019 screen-nx
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <switch.h>

#include <functional>
#include <string>
#include <vector>

namespace caps {

std::string dateToString(const CapsAlbumFileDateTime &date);
std::string entryToFileName(const CapsAlbumEntry &entry);
Result getThumbnail(u64 *width, u64 *height, const CapsAlbumEntry &entry, void *image, u64 image_size);
Result getImage(u64 *width, u64 *height, const CapsAlbumEntry &entry, void *image, u64 image_size);
Result getFile(const CapsAlbumEntry &entry, void *filebuf);
std::pair<Result, std::vector<CapsAlbumEntry>> getEntries(const CapsAlbumStorage &storage);
std::vector<CapsAlbumEntry> getAllEntries();
Result moveFile(const CapsAlbumEntry &entry);

class MovieReader {
public:
	MovieReader(const CapsAlbumEntry &entry);
	~MovieReader();
	u64 GetStreamSize();
	size_t Read(char *buffer, size_t max);

private:
	u32 lastBufferIndex = -1;
	u64 streamSize = 0;
	CapsAlbumEntry m_entry;
	u64 progress = 0;
	u64 stream = 0;
	unsigned char *readBuffer;
	u64 bufferSize = 0x40000;
	Result error = 0;
};

} // namespace caps
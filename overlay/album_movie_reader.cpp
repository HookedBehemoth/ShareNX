#include "album_movie_reader.hpp"

#include <tesla.hpp>

namespace album {

    namespace {

        constexpr const size_t AlbumMovieBufferSize = 0x40000;
        alignas(0x1000) u8 buffer[0x40000];

        u64 stream_id;
        u64 stream_size;
        u64 progress;
        int last_buffer_index;

    }

    Result Start(const CapsAlbumFileId &file_id) {
        stream_id = 0;
        R_TRY(capsaOpenAlbumMovieStream(&stream_id, &file_id));
        std::memset(buffer, 0, sizeof(buffer));

        progress = 0;
        last_buffer_index = -1;
        return capsaGetAlbumMovieStreamSize(stream_id, &stream_size);
    }

    Result Close() {
		Result rc = capsaCloseAlbumMovieStream(stream_id);
        stream_id = 0;
        stream_size = 0;
        progress = 0;
        last_buffer_index = -1;
        return rc;
    }

    u64 GetStreamSize() {
        return stream_size;
    }

    size_t Read(char *buffer, size_t size, size_t nitems, void*) {
        size_t max = size * nitems;

        /* Should we still read? */
		u64 remaining = stream_size - progress;
		if (remaining <= 0) {
			return 0;
		}
    
		int bufferIndex = progress / AlbumMovieBufferSize;
		u64 curOffset = progress % AlbumMovieBufferSize;
		u64 readSize = std::min({max, AlbumMovieBufferSize - curOffset, remaining});

        /* Do we need to fetch? */
		if (bufferIndex != last_buffer_index) {
			u64 actualSize = 0;
            
            /* Read movie data to temporary buffer. */
			if (R_FAILED(capsaReadMovieDataFromAlbumMovieReadStream(stream_id, bufferIndex * AlbumMovieBufferSize, buffer, AlbumMovieBufferSize, &actualSize))) {
                return 0;
            }
			last_buffer_index = bufferIndex;
		}

        /* Copy movie data to output. */
		char *startBuffer = buffer + curOffset;
		std::memcpy(buffer, startBuffer, readSize);
		progress += readSize;
        return readSize;
	}

}

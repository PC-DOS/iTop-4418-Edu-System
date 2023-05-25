#ifndef	__LIBMP3ENC_H__
#define	__LIBMP3ENC_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void * MP3_ENC_HANDLE;

enum {
	MODE_STEREO = 0,
	MODE_JOINT_STEREO,
	MODE_RESERVED,			// We doesn't supports this! */
	MODE_MONO,
};

//	BitRate Values
//	32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320
MP3_ENC_HANDLE	open_mp3enc		( void );
int				init_mp3enc		( MP3_ENC_HANDLE handle, int ch, int freq, int bitrate, int mode );
int				encode_mp3enc	( MP3_ENC_HANDLE handle, void *in_sample, int in_size, unsigned char *out_buf, int buf_size );
void			close_mp3enc	( MP3_ENC_HANDLE handle );

#ifdef __cplusplus
}
#endif

#endif	//	__LIBMP3ENC_H__

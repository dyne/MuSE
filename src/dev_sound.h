#ifndef __DEV_SOUND_H__
#define __DEV_SOUND_H__

#include <config.h>

#ifdef HAVE_JACK
#include <jack/jack.h>
#endif

#ifdef HAVE_PORTAUDIO
#include <portaudio.h>
#endif

#include <pipe.h>

#ifdef HAVE_PORTAUDIO
typedef struct {
  PaDeviceIndex id;
  PaStream *stream;
  PaStreamParameters input_params;
  PaStreamParameters output_params;  
  PaDeviceInfo *info;
  Pipe *pipe;
} PaDevInfo;

typedef struct {
   PaDevInfo *input;
   PaDevInfo *output;
} PaDevices;
///< the PortAudio device descriptor , this struct just group some data used by PortAudio framework
#endif

class SoundDevice {
 public:
  SoundDevice();
  ///< the SoundDevice class constructor
  ~SoundDevice();
  ///< the SoundDevice class destructor

  /**
     Tries to open the sound device for read and/or write
     if full-duplex is requested but not supported, it returns error
     and must be called again to fallback on half-duplex mode

     @param read true if device is opened for reading audio
     @param write true if device is opened for writing audio
     @return true in case of success, false otherwise
  */
  bool open(bool read, bool write);
  ///< open the sound device

  bool input(bool state); ///< activate sound input
  bool output(bool state); ///< activate sound output

  void close(); ///< close the sound device

  int read(void *buf, int len); ///< reads audio data from the device in a buffer, len is samples

  int write(void *buf, int len); ///< writes audio data from a buffer to the device, len is samples
  void flush_input();
  void flush_output();

#ifdef HAVE_JACK
  bool jack;
  bool jack_in;
  bool jack_out;

  jack_default_audio_sample_t *get_jack_in_buf() { return jack_in_buf; };
  void set_jack_in_buf(jack_default_audio_sample_t *arg) { jack_in_buf = arg; };

  jack_default_audio_sample_t *get_jack_out_buf() { return jack_out_buf; };
  void set_jack_out_buf(jack_default_audio_sample_t *arg) { jack_out_buf = arg; };

  jack_port_t *get_jack_in_port() { return jack_in_port; };
  void set_jack_in_port(jack_port_t *arg) { jack_in_port = arg; };

  jack_port_t *get_jack_out_port() { return jack_out_port; };
  void set_jack_out_port(jack_port_t *arg) { jack_out_port = arg; };

  Pipe *get_jack_out_pipe() { return jack_out_pipe; };
  void set_jack_out_pipe(Pipe *arg) { jack_out_pipe = arg; };

  Pipe *get_jack_in_pipe() { return jack_in_pipe; };
  void set_jack_in_pipe(Pipe *arg) { jack_in_pipe = arg; };

  jack_client_t *get_jack_client() { return jack_client; };
  void set_jack_client(jack_client_t *arg) { jack_client = arg; };

#endif

#ifdef HAVE_PORTAUDIO
  /* TODO - should be private */
  PaDevInfo input_device; ///< portaudio input device
  PaDevInfo output_device; ///< portaudio output device
#endif


 private:
#ifdef HAVE_PORTAUDIO
  bool pa_open(bool state,int mode);
  PaError pa_real_open(int mode);

  PaError err;

  PaDevices pa_dev;
  int pa_mode; ///< a switch to represent portaudio mode currently using (for noaudio,input,output or both) 
#define PaNull 0
#define PaInput 1
#define PaOutput 2
#endif

#ifdef HAVE_JACK
  Pipe *jack_in_pipe;
  Pipe *jack_out_pipe;
  jack_client_t *jack_client;
  jack_port_t *jack_in_port;
  jack_port_t *jack_out_port;
  jack_default_audio_sample_t *jack_in_buf;
  jack_default_audio_sample_t *jack_out_buf;
  size_t jack_sample_size;
  int jack_samplerate;
#endif
};

#endif // __DEV_SOUND_H__

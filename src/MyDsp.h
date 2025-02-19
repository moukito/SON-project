#ifndef MYDSP_H
#define MYDSP_H

#include "Arduino.h"
#include "AudioStream.h"
#include "Audio.h"

class MyDsp final : public AudioStream {
	public:
		MyDsp();

		virtual ~MyDsp();

		void update() override;
};

#endif //MYDSP_H
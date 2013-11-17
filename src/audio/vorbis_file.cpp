#include "i_audio.h"

VorbisFile::VorbisFile( AutoFile File )
: mSource(File)
, mStatus(St_None)
{
}

bool VorbisFile::Init()
{
	if(!mSource.get())return false;
	ov_callbacks Callbacks;
	Callbacks.seek_func=NULL;
	Callbacks.tell_func=NULL;
	Callbacks.read_func=&VorbisFile::ReadFunc;
	Callbacks.close_func=&VorbisFile::CloseFunc;
	if(ov_open_callbacks(this,&mVorbisFile,NULL,0,Callbacks)<0)return false;
	mStatus=St_Inited;
	if(!ReadInfo())return false;
	mBuffer.reset(new AudioBuffer(mInfo.channels));
	return FillBufferIfNeeded();
}

bool VorbisFile::ReadInfo()
{
	char **ptr=ov_comment(&mVorbisFile,-1)->user_comments;
	vorbis_info *vi=ov_info(&mVorbisFile,-1);
	mInfo=*vi;
	while(*ptr){
		LOG("%s\n",*ptr);
		++ptr;
	}
	LOG("\nBitstream is %d channel, %ldHz\n",mInfo.channels,mInfo.rate);
	// LOG("\nDecoded length: %ld samples\n",(long)ov_pcm_total(&mVorbisFile,-1)); no seek support right now
	LOG("Encoded by: %s\n\n",ov_comment(&mVorbisFile,-1)->vendor);
	bool const IsRateSupported=AudioPlayer::Get().IsSampleRateSupported(mInfo.rate);
	LOG("Sample rate is %ssupported",IsRateSupported?"":"NOT ");
	return IsRateSupported;
}

bool VorbisFile::ReadBlock()
{
	static const size_t MaxSamples=4096;
	float** Buffer;
	int CurrentSection;
	long SamplesRead=ov_read_float(&mVorbisFile,&Buffer,MaxSamples,&CurrentSection);
	if(SamplesRead<0)
	{
		LOG("Vorbisfile: error %d\n",SamplesRead);
		if(SamplesRead==OV_EBADLINK){
			LOG("Corrupt bitstream section! Exiting.\n");
			return false;
		}
	}
	else if(SamplesRead)
	{
		mBuffer->Write(Buffer,SamplesRead);
	}
	return !!SamplesRead;
}

size_t VorbisFile::ReadFunc( void* Buffer,size_t Size, size_t Count, void* This )
{
	VorbisFile* Vf=static_cast<VorbisFile*>(This);
	File& InFile=*(Vf->mSource);
	std::string Buf;
	size_t const Remaining=InFile.GetSize()-InFile.GetPosition();
	size_t Len=std::min<size_t>(Size*Count,Remaining);
	if(!Len)
	{
		ov_clear(&Vf->mVorbisFile);
		return 0;
	}
	if(!InFile.Read(Buf,Len))
		return -1;
	memcpy(Buffer,Buf.data(),Buf.size());
	return Buf.size();
}

int VorbisFile::CloseFunc( void* This )
{
	VorbisFile* Vf=static_cast<VorbisFile*>(This);
	Vf->mSource.reset();
	Vf->mStatus=St_Closed;
	return 0;
}

VorbisFile::~VorbisFile()
{
}

std::auto_ptr<VorbisFile> VorbisFile::Create( AutoFile File )
{
	std::auto_ptr<VorbisFile> Ret(new VorbisFile(File));
	if(!Ret->Init())Ret.reset();
	return Ret;
}

bool VorbisFile::FillBufferIfNeeded()
{
	if(!mBuffer.get()||!mSource.get())
		return false;
	if(mBuffer->GetSize()>=TargetBufferSize)
		return false;
	while(mStatus==St_Inited&&mBuffer->GetSize()<2*TargetBufferSize&&ReadBlock())
		;
	return true;
}

AudioBuffer& VorbisFile::GetBuffer()
{
	return *mBuffer;
}

bool VorbisFile::IsFinished() const
{
	return mStatus==St_Closed&&mBuffer->GetSize()==0;
}

const size_t VorbisFile::TargetBufferSize=4096;

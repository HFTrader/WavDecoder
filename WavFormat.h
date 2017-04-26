#pragma once
#include <stdint.h>
#include "FileUtils.h"

// http://www.topherlee.com/software/pcm-tut-wavformat.html
struct  WAV_HEADER
{
    /* RIFF Chunk Descriptor */
    uint8_t         RIFF[4];        // RIFF Header Magic header
    uint32_t        ChunkSize;      // RIFF Chunk Size
    uint8_t         WAVE[4];        // WAVE Header
    /* "fmt" sub-chunk */
    uint8_t         fmt[4];         // FMT header
    uint32_t        Subchunk1Size;  // Size of the fmt chunk
    uint16_t        AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t        NumOfChan;      // Number of channels 1=Mono 2=Sterio
    uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
    uint32_t        bytesPerSec;    // bytes per second
    uint16_t        blockAlign;     // 2=16-bit mono, 4=16-bit stereo
    uint16_t        bitsPerSample;  // Number of bits per sample
    /* "data" sub-chunk */
    uint8_t         Subchunk2ID[4]; // "data"  string
    uint32_t        Subchunk2Size;  // Sampled data length
} __attribute__((packed));

bool encodeWavFormat( const SampleArray& wav, ByteArray& bytes, uint32_t SAMPLE_HZ ) 
{
    const uint32_t num_channels = 1;
    const uint32_t bits_per_sample = 16;

    uint32_t datasize = ((num_channels*bits_per_sample)/8)*wav.size();
    uint32_t hdrsize = sizeof(struct WAV_HEADER);

    struct WAV_HEADER hdr;
    memset( &hdr, 0, hdrsize );
    hdr.ChunkSize = 2*wav.size() - 8;
    memcpy( hdr.RIFF, "RIFF", 4 );
    memcpy( hdr.WAVE, "WAVE", 4 );
    memcpy( hdr.fmt, "fmt ", 4 );

    hdr.Subchunk1Size = bits_per_sample;
    hdr.AudioFormat = 1;
    hdr.NumOfChan = num_channels;
    hdr.SamplesPerSec = SAMPLE_HZ;
    hdr.bytesPerSec = (bits_per_sample*SAMPLE_HZ*num_channels)/8;
    hdr.blockAlign = (bits_per_sample*num_channels)/8;
    hdr.bitsPerSample = bits_per_sample;
    memcpy( hdr.Subchunk2ID, "data", 4 );
    hdr.Subchunk2Size = ((num_channels*bits_per_sample)/8)*wav.size() - sizeof(struct WAV_HEADER);

    bytes.resize( datasize + sizeof(struct WAV_HEADER) );
    memcpy( &bytes[0], &hdr, hdrsize );
    memcpy( &bytes[hdrsize], &wav[0], datasize );

    return true;
}

// Everything here is hardcoded for 1 channel, 16 bits
bool decodeWavFormat( const ByteArray& bytes, SampleArray& wav, double& freq_hz ) 
{
    struct WAV_HEADER* hdr( (struct WAV_HEADER*)&bytes[0] );
    uint32_t num_samples = (hdr->ChunkSize+8)/2;
    uint32_t num_channels = hdr->NumOfChan;
    uint32_t bits_per_sample = hdr->Subchunk1Size;
    freq_hz = hdr->SamplesPerSec;
    if ( (num_channels != 1) || ( bits_per_sample != 16 ) || ( hdr->AudioFormat != 1 ) )
        return false;

    uint32_t datasize = ((num_channels*bits_per_sample)/8)*num_samples;
    uint32_t hdrsize = sizeof(struct WAV_HEADER);
    
    wav.resize( num_samples );
    memcpy( &wav[0], &bytes[hdrsize], datasize );
    return true;
}

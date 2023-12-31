/*
 * Modification History
 *
 * 2000-December-21		Jason Rohrer
 * Created. 
 *
 * 2001-January-15	Jason Rohrer
 * Added an overridden copy() method.  
 *
 * 2001-January-19	Jason Rohrer
 * Fixed so that filter( filter, channelNum ) could be called on
 * this class type. 
 *
 * 2001-January-31		Jason Rohrer
 * Fixed a bug in getRGBABytes().
 *
 * 2001-September-15		Jason Rohrer
 * Added a constructor that copies data from an Image.
 *
 * 2006-September-7		Jason Rohrer
 * Optimized constructor.
 *
 * 2010-February-23		Jason Rohrer
 * Changed default alpha behavior to white instead of black.
 *
 * 2010-February-24		Jason Rohrer
 * Optimized with a static function to avoid needless image copying.
 *
 * 2011-April-5     Jason Rohrer
 * Fixed float-to-int conversion.
 * See:  http://www.mega-nerd.com/FPcast/  
 *
 * 2023-June-8     Jason Rohrer
 * Added a function for producing an Image from an 8-bit color byte array.
 */
 
 
#ifndef RGBA_IMAGE_INCLUDED
#define RGBA_IMAGE_INCLUDED

#include <stdio.h> 
#include <string.h>
#include <math.h>
 
#include "Image.h"
 
/**
 * An RGBA extension of Image.
 *
 * @author Jason Rohrer 
 */
class RGBAImage : public Image { 
	
	public:
		
		/**
		 * Constructs an RGBA image.  All channels are initialized to 0.
		 *
		 * @param inWidth width of image in pixels.
		 * @param inHeight height of image in pixels.
		 */
		RGBAImage( int inWidth, int inHeight );
		
		
		/**
		 * Constructs an RGBAImage by copying a given image.
		 * The image data is truncated or expaned (with black color channels 
         * and white alpha) to fit the 4 channel RGBA model,
		 * and any selection in inImage is ignored.
		 *
		 * @param inImage the image to copy.  Copied internally, so must be
		 *   destroyed by the caller.
		 */
		RGBAImage( Image *inImage );
		
		
		/**
		 * Gets the pixel data from this image as a byte array.
		 *
		 * @return a byte array containing pixel data for this image.
		 *   Stored in row-major order, with each pixel represented
		 *   by 4 bytes in the order RGBA.
		 *   Must be destroyed by caller.
		 */
		virtual unsigned char *getRGBABytes();

		// get RGBA a bytes from an image without converting it to an RGBA
        // image first (faster---found with profiler)
        //
        // MUST be a 3- or 4-channel image
        static unsigned char *getRGBABytes( Image *inImage );
        
        
        // contstructs an image from 8-bit color data
        // channels must be interleaved in inBytes
        // for example, RGBRGBRGB...
        // Works with any number of channels, but code is optimized
        // for 3- or 4-channel images.
        static Image *getImageFromBytes( unsigned char *inBytes,
                                         int inWidth, int inHeight,
                                         int inNumChannels );
        

		
		// overrides the Image::filter function
		virtual void filter( ChannelFilter *inFilter );
		virtual void filter( ChannelFilter *inFilter, int inChannel );
		
		
		// overrides the Image::copy function
		// since the function is non-virtual in the parent class,
		// the pointer type should determine which function gets called.
		RGBAImage *copy();
		
	};



inline RGBAImage::RGBAImage( int inWidth, int inHeight )
	: Image( inWidth, inHeight, 4 ) {
	
	}



inline RGBAImage::RGBAImage( Image *inImage )
    // Only init our image's channels to black of inImage doesn't have enough
    // channels.
    // This saves time if inImage has 4 or more channels.
    // Optimization found with profiler.
	: Image( inImage->getWidth(), inImage->getHeight(), 4,
             ( inImage->getNumChannels() < 4 ) ) {
	
	int minNumChannels = 4;
	if( inImage->getNumChannels() < minNumChannels ) {
		minNumChannels = inImage->getNumChannels();
		}

	// if inImage does not have at least 4 channels,
	// leave our additional channels black
	// if inImage has extra channels, skip them

	// copy channels from inImage
	for( int c=0; c<minNumChannels; c++ ) {
		double *inChannel = inImage->getChannel( c );
		
		memcpy( mChannels[c], inChannel, mNumPixels * sizeof( double ) ); 
		}

    if( minNumChannels < 4 ) {
        // no alpha channel in inImage
        
        // set a white one
        double *alpha = mChannels[3];
        
        for( int i=0; i<mNumPixels; i++ ) {
            alpha[i] = 1.0;
            }
        }
    
	}

	
/*	
inline unsigned char *RGBAImage::getRGBABytes() {
	int numBytes = mNumPixels * 4; 
	unsigned char *bytes = new unsigned char[ numBytes ];
	int pixel = 0;
	for( int i=0; i<numBytes; i=i+4 ) {
		bytes[i] = (unsigned char)( mChannels[0][pixel] * 255 );
		bytes[i + 1] = (unsigned char)( mChannels[1][pixel] * 255 );
		bytes[i + 2] = (unsigned char)( mChannels[2][pixel] * 255 );
		bytes[i + 3] = (unsigned char)( mChannels[3][pixel] * 255 );
		pixel++;
		}
	return bytes;
	}
*/

// optimized version
inline unsigned char *RGBAImage::getRGBABytes() {
    return RGBAImage::getRGBABytes( this );
    
    /*
	int numBytes = mNumPixels * 4; 
	unsigned char *bytes = new unsigned char[ numBytes ];

    double *channelZero = mChannels[0];
    double *channelOne = mChannels[1];
    double *channelTwo = mChannels[2];
    double *channelThree = mChannels[3];
    
    
    register int i = 0;
    for( int p=0; p<mNumPixels; p++ ) {
		bytes[i++] = (unsigned char)( channelZero[p] * 255 );
		bytes[i++] = (unsigned char)( channelOne[p] * 255 );
		bytes[i++] = (unsigned char)( channelTwo[p] * 255 );
		bytes[i++] = (unsigned char)( channelThree[p] * 255 );
		}
	return bytes;
    */
	}


inline unsigned char *RGBAImage::getRGBABytes( Image *inImage ) {
    int numPixels = inImage->getWidth() * inImage->getHeight();
    
    int numChannels = inImage->getNumChannels();
    

    int numBytes = numPixels * 4; 
	unsigned char *bytes = new unsigned char[ numBytes ];

    double *channelZero = inImage->getChannel( 0 );
    double *channelOne = inImage->getChannel( 1 );
    double *channelTwo = inImage->getChannel( 2 );
    
    // double-coded branch here to make it faster 
    // (avoid a switch inside the loop
    if( numChannels > 3 ) {    
        double *channelThree = inImage->getChannel( 3 );
    
        register int i = 0;
        for( int p=0; p<numPixels; p++ ) {
            bytes[i++] = (unsigned char)( lrint( channelZero[p] * 255 ) );
            bytes[i++] = (unsigned char)( lrint( channelOne[p] * 255 ) );
            bytes[i++] = (unsigned char)( lrint( channelTwo[p] * 255 ) );
            bytes[i++] = (unsigned char)( lrint( channelThree[p] * 255 ) );
            }
        }
    else {
        register int i = 0;
        for( int p=0; p<numPixels; p++ ) {
            bytes[i++] = (unsigned char)( lrint( channelZero[p] * 255 ) );
            bytes[i++] = (unsigned char)( lrint( channelOne[p] * 255 ) );
            bytes[i++] = (unsigned char)( lrint( channelTwo[p] * 255 ) );
            bytes[i++] = 255;  // default alpha
            }
        }
    
    
	return bytes;
    }


	

inline void RGBAImage::filter( ChannelFilter *inFilter ) {
	// different from standard Image implementation in that we
	// skip the alpha channel when filtering
	for( int i=0; i<mNumChannels-1; i++ ) {
		Image::filter( inFilter, i );
		}	
	}



inline void RGBAImage::filter( ChannelFilter *inFilter, int inChannel ) {
	// provided as a bridge to the Image function, just to
	// get around compile problems.
	Image::filter( inFilter, inChannel );	
	}



inline RGBAImage *RGBAImage::copy() {
	RGBAImage *copiedImage = new RGBAImage( mWide, mHigh );
	copiedImage->paste( this );
	
	return copiedImage;
	}
	


inline Image *RGBAImage::getImageFromBytes( unsigned char *inBytes,
                                            int inWidth, int inHeight,
                                            int inNumChannels ) {

    Image *im = new Image( inWidth, inHeight, inNumChannels, false );

    if( inNumChannels == 3 ) {
        // optimized code for 3 channels

        // code roughly duplicated from gameSDL.cpp getScreenRegionInternal

        double *channelOne = im->getChannel( 0 );
        double *channelTwo = im->getChannel( 1 );
        double *channelThree = im->getChannel( 2 );
        
        int outputRow = 0;
        for( int y=0; y<inHeight; y++ ) {
            for( int x=0; x<inWidth; x++ ) {
                
                int outputPixelIndex = outputRow * inWidth + x;
                
                
                int regionPixelIndex = y * inWidth + x;
                int byteIndex = regionPixelIndex * 3;
                
                // optimization found:  should unroll this loop over 3 channels
                // divide by 255, with a multiply
                channelOne[outputPixelIndex] = 
                    inBytes[ byteIndex++ ] * 0.003921569;
                channelTwo[outputPixelIndex] = 
                    inBytes[ byteIndex++ ] * 0.003921569;
                channelThree[outputPixelIndex] = 
                    inBytes[ byteIndex++ ] * 0.003921569;
                }
            outputRow++;
            }
        }
    else if( inNumChannels == 4 ) {
        // optimized code for 3 channels

        // code roughly duplicated from gameSDL.cpp getScreenRegionInternal

        double *channelOne = im->getChannel( 0 );
        double *channelTwo = im->getChannel( 1 );
        double *channelThree = im->getChannel( 2 );
        double *channelFour = im->getChannel( 3 );
        
        int outputRow = 0;
        for( int y=0; y<inHeight; y++ ) {
            for( int x=0; x<inWidth; x++ ) {
                
                int outputPixelIndex = outputRow * inWidth + x;
                
                
                int regionPixelIndex = y * inWidth + x;
                int byteIndex = regionPixelIndex * 4;
                
                // optimization found:  should unroll this loop over 4 channels
                // divide by 255, with a multiply
                channelOne[outputPixelIndex] = 
                    inBytes[ byteIndex++ ] * 0.003921569;
                channelTwo[outputPixelIndex] = 
                    inBytes[ byteIndex++ ] * 0.003921569;
                channelThree[outputPixelIndex] = 
                    inBytes[ byteIndex++ ] * 0.003921569;
                channelFour[outputPixelIndex] = 
                    inBytes[ byteIndex++ ] * 0.003921569;
                }
            outputRow++;
            }
        }
    else {
        // for other numbers of channels
        // dynamic code

        double **channels = new double*[inNumChannels];

        for( int i=0; i<inNumChannels; i++ ) {
            channels[i] = im->getChannel( i );
            }
        
        int outputRow = 0;
        
        for( int y=0; y<inHeight; y++ ) {
            for( int x=0; x<inWidth; x++ ) {
                
                int outputPixelIndex = outputRow * inWidth + x;
                
                
                int regionPixelIndex = y * inWidth + x;
                int byteIndex = regionPixelIndex * inNumChannels;
                
                // this is less-optimized than 3-channel code
                // because we don't unroll inner loop
                for( int i=0; i<inNumChannels; i++ ) {
                    channels[i][outputPixelIndex] = 
                        inBytes[ byteIndex++ ] * 0.003921569;
                    }
                }
            outputRow++;
            }
        
        delete [] channels;
        }
    

    
    return im;
    }


		
#endif

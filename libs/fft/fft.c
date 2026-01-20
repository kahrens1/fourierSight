#include"fft.h"
#include"fft_tables.h"
#include<stdio.h>


static void apply_br(fft_instance_t *fft_init, complex_t *data);


_Bool fft_init(fft_instance_t *fft_init, uint16_t numPoints){

    fft_init->numPoints = numPoints;
   
    switch(numPoints){
        
        #ifdef FFT_USE_32_POINT
            case 32:
                fft_init->numStages = 5;     
                #ifdef FFT_USE_BIT_REV_LUT
                    fft_init->bitRevTable = bitRev32;
                #endif 
                #ifdef FFT_USE_TFS_LUT  
                    fft_init->tfs = tfs32;
                #endif 
                // #ifndef FFT_USE_BIT_REV_LUT
                // Add code in these sections to compute values instead of using LUT
                // #endif
                // #ifndef FFT_USE_TFS_LUT
                // Add code in these sections to compute values instead of using LUT
                // #endif
            break;
        #endif 
        #ifdef FFT_USE_64_POINT
            case 64:
                fft_init->numStages = 6;
                #ifdef FFT_USE_BIT_REV_LUT
                    fft_init->bitRevTable = bitRev64;
                #endif 
                #ifdef FFT_USE_TFS_LUT  
                    fft_init->tfs = tfs64;
                #endif 
                // #ifndef FFT_USE_BIT_REV_LUT
                // Add code in these sections to compute values instead of using LUT
                // #endif
                // #ifndef FFT_USE_TFS_LUT
                // Add code in these sections to compute values instead of using LUT
                // #endif
            break;
        #endif
        #ifdef FFT_USE_128_POINT
            case 128:
                fft_init->numStages = 7;
                #ifdef FFT_USE_BIT_REV_LUT
                    fft_init->bitRevTable = bitRev128;
                #endif 
                #ifdef FFT_USE_TFS_LUT  
                    fft_init->tfs = tfs128;
                #endif
                // #ifndef FFT_USE_BIT_REV_LUT
                // Add code in these sections to compute values instead of using LUT
                // #endif
                // #ifndef FFT_USE_TFS_LUT
                // Add code in these sections to compute values instead of using LUT
                // #endif
            break;
        #endif
        #ifdef FFT_USE_256_POINT
        case 256:
             fft_init->numStages = 8;
            #ifdef FFT_USE_BIT_REV_LUT
                fft_init->bitRevTable = bitRev256;
            #endif  
            #ifdef FFT_USE_TFS_LUT  
                fft_init->tfs = tfs256;
            #endif
            // #ifndef FFT_USE_BIT_REV_LUT
            // Add code in these sections to compute values instead of using LUT
            // #endif
            // #ifndef FFT_USE_TFS_LUT
            // Add code in these sections to compute values instead of using LUT
            // #endif
        break;
        #endif
        #ifdef FFT_USE_512_POINT
        case 512:
             fft_init->numStages = 9;
            #ifdef FFT_USE_BIT_REV_LUT
                fft_init->bitRevTable = bitRev512;
            #endif 
            #ifdef FFT_USE_TFS_LUT  
                fft_init->tfs = tfs512;
            #endif
            // #ifndef FFT_USE_BIT_REV_LUT
            // Add code in these sections to compute values instead of using LUT
            // #endif
            // #ifndef FFT_USE_TFS_LUT
            // Add code in these sections to compute values instead of using LUT
            // #endif
        break; 
        #endif

        #ifdef FFT_USE_1024_POINT
        case 1024:
             fft_init->numStages = 10;
            #ifdef FFT_USE_BIT_REV_LUT
                fft_init->bitRevTable = bitRev1024;
            #endif 
            #ifdef FFT_USE_TFS_LUT  
                fft_init->tfs = tfs1024;
            #endif
             // #ifndef FFT_USE_BIT_REV_LUT
                // Add code in these sections to compute values instead of using LUT
             // #endif
            // #ifndef FFT_USE_TFS_LUT
            // Add code in these sections to compute values instead of using LUT
            // #endif
        break; 
        #endif
        default: 
            return false;
        break;
    
    }
    return true;
}


void fft_compute(fft_instance_t *fft_init, complex_t *data){
    // Loop over stages
    uint8_t numBlocks;
    uint16_t numBfs;
    uint16_t bf_span;
    uint16_t blk_step;
    uint8_t bf_step;
    complex_t tf;
    complex_t temp;
    uint16_t in1;
    uint16_t in2;
    uint32_t tf_idx;
    uint16_t blk_ptr;

    apply_br(fft_init,data);

    for(uint8_t stage_idx = 0; stage_idx < fft_init->numStages; stage_idx++){
        numBfs = 1 << stage_idx;
        bf_span = 1 << stage_idx;
        blk_step = 1 << (stage_idx + 1);
        numBlocks = fft_init->numPoints >> (stage_idx + 1);
        // bf_step = 1;
         printf("Stage %d\n",stage_idx);
        //Loop over blocks
        for(uint16_t blk_idx = 0; blk_idx < numBlocks; blk_idx++){
            blk_ptr = blk_idx*blk_step;
            printf("Block Index %d, Block Pointer %d\n",blk_idx,blk_ptr);
            // Loop over BFs in the block
            for(uint16_t bf_idx = 0; bf_idx < numBfs; bf_idx++ ){
                printf("BF Index %d\n",bf_idx);
                tf_idx = (bf_idx*fft_init->numPoints) / blk_step;
                tf =  fft_init->tfs[tf_idx];
                printf("TF: Real = %f, TF: Imag: %f\n",tf.re,tf.im);
                in1 = blk_ptr + bf_idx;  
                in2 = in1 + bf_span;
                temp = data[in1];
                data[in1] = complex_add(data[in1],complex_mult(data[in2], tf));
                data[in2] = complex_sub(temp,complex_mult(data[in2],tf));

            }

        }

    }
}

static void apply_br(fft_instance_t *fft_init, complex_t *data){
    //Do as pairs save 2 temporary values and re-assignt
    uint16_t br_idx;
    complex_t temp;
    for(uint16_t i = 0; i < fft_init->numPoints; i++){  
        br_idx = fft_init->bitRevTable[i];
        if(i < br_idx){
            temp = data[i];
            data[i] = data[br_idx];
            data[br_idx] = temp;
        }
    }
}


//TODO: Add function to compute the magnitude


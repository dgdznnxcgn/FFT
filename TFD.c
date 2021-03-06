#include <math.h>
//#include <fcntl.h> // for open
//#include <unistd.h> // for close
#include <stdio.h>
#include <stdlib.h>
#include <string.h> //memcpy
#include "wave.h"

#define PI 3.141592654

void formatowave (FILE *archivo, wave *w);
void imprimirFormato (wave *wav);
void escribirArchivo(FILE *archivoEscritura, wave *header);
void Multiplicacion (FILE *archivo, wave *in, wave *out);
void Trasformadafourier(FILE *archivo, wave *s);
/*----------------------------------*/
void CrearWaveFormatMono2Stereo(short *data, wave *old, wave *header, int num_channels, long num_samples, int bits_per_sample, long sample_rate);
void BigEndianLong2LittleEndianChar(unsigned char *a, unsigned long in, int tam);
/*--------------------------------------------*/
void Escalamiento(double *w, wave *entrada);

int main(int argc, char **argv){
	FILE *archivoEntrada1, *archivoEscritura;
	wave wav1, wav2;
	wave *conv;	 
	archivoEntrada1=fopen("uno.wav","rb");
	archivoEscritura=fopen("salida.wav","wb");
	//archivoEntrada1=fopen(argv[1],"rb");
	//archivoEscritura=fopen(argv[2],"wb");	

	if (archivoEntrada1!=NULL && archivoEscritura!=NULL) 
 	{
		formatowave(archivoEntrada1, &wav1);	
		//imprimirFormato(&wav1);	
		Trasformadafourier(archivoEscritura, &wav1);
		imprimirFormato(&wav1);
		escribirArchivo(archivoEscritura, &wav1);
		//Multiplicacion(archivoEscritura,&wav1,&wav2);
		//escribirArchivo(archivoEscritura, conv);
		//imprimirFormato(conv);

		fclose(archivoEntrada1);
		fclose(archivoEscritura);
	}else
 	printf("No se pudo generar el archivo\n");

}

void Trasformadafourier(FILE *archivo, wave *s){
	int k,n,N,i;
	N = s->num_samples;
	double *real = malloc(sizeof(double)*s->num_samples);
	double *imag = malloc(sizeof(double)*s->num_samples);
	short *data = malloc(sizeof(short)*(2*s->num_samples));
 	i=0;
	for(k=0; k < s->num_samples; k++){
		real[k]=0;
		imag[k]=0;
		for(n=0; n < s->num_samples; n++){
			real[k] += s->muestras[n]*cos(2*PI*k*n/N);
			imag[k] += s->muestras[n]*sin(2*PI*k*n/N);
			//printf("k[%d]n[%d]Cos:%lf\n",k, n, s->muestras[n]*cos(2*PI*k*n/N));
			//printf("Sen:%lf\n", sin(2*PI*k*n/N));
		}
		//real[k] = real[k]/N;
		//imag[k] = -imag[k]/N;
		printf("real[%lf]imaginaria[%lf]\n", real[k], imag[k]);
		data[i] = real[k]/N;
		i++;
		data[i] = -imag[k]/N;
		i++;
	}
	CrearWaveFormatMono2Stereo(data, s, s, 2, s->num_samples, s->bits_per_sample, s->sample_rate);  	
}

void escribirArchivo(FILE *archivoEscritura, wave *header){
	fwrite(header->riff, sizeof(header->riff),1,archivoEscritura);
	fwrite(header->overall_sizeb, sizeof(header->overall_sizeb),1,archivoEscritura);
	fwrite(header->wave, sizeof(header->wave),1,archivoEscritura);
	fwrite(header->fmt_chunk_marker, sizeof(header->fmt_chunk_marker),1,archivoEscritura);
	fwrite(header->length_of_fmtb, sizeof(header->length_of_fmtb),1,archivoEscritura);
	fwrite(header->format_typeb, sizeof(header->format_typeb),1,archivoEscritura);
	fwrite(header->channelsb, sizeof(header->channelsb),1,archivoEscritura);
	fwrite(header->sample_rateb, sizeof(header->sample_rateb),1,archivoEscritura);
	fwrite(header->byterateb, sizeof(header->byterateb),1,archivoEscritura);
	fwrite(header->block_alignb, sizeof(header->block_alignb),1,archivoEscritura);
	fwrite(header->bits_per_sampleb, sizeof(header->bits_per_sampleb),1,archivoEscritura);
	fwrite(header->data_chunk_header, sizeof(header->data_chunk_header),1,archivoEscritura);
	fwrite(header->data_sizeb, sizeof(header->data_sizeb),1,archivoEscritura);
	fwrite(header->muestras, sizeof(short)*2*header->num_samples,1,archivoEscritura);
	//fwrite(header->muestras, sizeof(header->muestras),1,archivoEscritura);
	fwrite(header->end, sizeof(header->end),1,archivoEscritura);
}

void imprimirFormato (wave *wav){
	int i;
	printf("(%c)(%c)(%c)(%c):\n", wav->riff[0], wav->riff[1], wav->riff[2], wav->riff[3]);
	printf("(%lu):\n", wav->overall_size);
	printf("(%c)(%c)(%c)(%c):\n", wav->wave[0], wav->wave[1], wav->wave[2], wav->wave[3]);
	printf("(%c)(%c)(%c):\n", wav->fmt_chunk_marker[0], wav->fmt_chunk_marker[1], wav->fmt_chunk_marker[2]);
	printf("length_of_fmt(%lu):\n", wav->length_of_fmt);
	printf("format_type(%u):\n", wav->format_type);
	printf("channels(%u):\n", wav->channels);
	printf("sample rate:(%u):\n", wav->sample_rate);
	printf("byterate(%u):\n", wav->byterate);
	printf("block_align(%u):\n", wav->block_align);
	printf("bits_per_sample(%u):\n", wav->bits_per_sample);
	printf("(%c)(%c)(%c)(%c):\n", wav->data_chunk_header[0], wav->data_chunk_header[1], wav->data_chunk_header[2], wav->data_chunk_header[3]);
	printf("data_size(%ld bytes):\n", wav->data_size);
	printf("num_samples(%lu):\n", wav->num_samples);
	printf("size(%ld):\n", wav->size_of_each_sample);
	for(i=0; i<wav->num_samples; i++){
		printf("(%d:%04x)\n",i, wav->muestras[i]);
		}
	for(i=0; i<74; i++){
		//printf("|%d:%02x", i, wav->end[i]);
	}//printf("|\n");
}

void formatowave (FILE *archivoEntrada, wave *header){
	unsigned char buffer4[4];
 	unsigned char buffer2[2];
 	int i;
	if (archivoEntrada!=NULL) 
 	{
 		fread(header->riff, sizeof(header->riff), 1, archivoEntrada);
 		fread(header->overall_sizeb, sizeof(header->overall_sizeb), 1, archivoEntrada);
 		 header->overall_size  = header->overall_sizeb[0] | 
						(header->overall_sizeb[1]<<8) | 
						(header->overall_sizeb[2]<<16) | 
						(header->overall_sizeb[3]<<24);
 		fread(header->wave, sizeof(header->wave), 1, archivoEntrada);
 		fread(header->fmt_chunk_marker, sizeof(header->fmt_chunk_marker), 1, archivoEntrada);
 		fread(header->length_of_fmtb, sizeof(header->length_of_fmtb), 1, archivoEntrada);
 		 header->length_of_fmt = header->length_of_fmtb[0] |
							(header->length_of_fmtb[1] << 8) |
							(header->length_of_fmtb[2] << 16) |
							(header->length_of_fmtb[3] << 24);
 		fread(header->format_typeb, sizeof(header->format_typeb), 1, archivoEntrada);
 		 header->format_type = header->format_typeb[0] | (header->format_typeb[1] << 8);
 		fread(header->channelsb, sizeof(header->channelsb), 1, archivoEntrada);
 		 header->channels = header->channelsb[0] | (header->channelsb[1] << 8);
 		fread(header->sample_rateb, sizeof(header->sample_rateb), 1, archivoEntrada);
 		 header->sample_rate = header->sample_rateb[0] |
						(header->sample_rateb[1] << 8) |
						(header->sample_rateb[2] << 16) |
						(header->sample_rateb[3] << 24);
 		fread(header->byterateb, sizeof(header->byterateb), 1, archivoEntrada); 
 		 header->byterate  = header->byterateb[0] |
						(header->byterateb[1] << 8) |
						(header->byterateb[2] << 16) |
						(header->byterateb[3] << 24);
 		fread(header->block_alignb, sizeof(header->block_alignb), 1, archivoEntrada);
 		 header->block_align = header->block_alignb[0] |
					(header->block_alignb[1] << 8);
 		fread(header->bits_per_sampleb, sizeof(header->bits_per_sampleb), 1, archivoEntrada);
 		 header->bits_per_sample = header->bits_per_sampleb[0] |
					(header->bits_per_sampleb[1] << 8); 
 		fread(header->data_chunk_header, sizeof(header->data_chunk_header), 1, archivoEntrada);
 		fread(header->data_sizeb, sizeof(header->data_sizeb), 1, archivoEntrada);
		  header->data_size = header->data_sizeb[0] |
				(header->data_sizeb[1] << 8) |
				(header->data_sizeb[2] << 16) | 
				(header->data_sizeb[3] << 24 );
 		header->num_samples = (8 * header->data_size) / (header->channels * header->bits_per_sample);
 		header->size_of_each_sample = (header->channels * header->bits_per_sample) / 8;
 		header->muestras = malloc (sizeof(short)*header->num_samples);
 		for(i=0; i<header->num_samples; i++){
	 		fread(&header->muestras[i], sizeof(short), 1, archivoEntrada);
			//printf("(%x):\n", header->muestras[i]);
		}
		for(i=0; i<74; i++){
			fread(&header->end[i], sizeof(char), 1, archivoEntrada);
			//printf("%x\n", header->end[i]);
		}
	}else{
		printf("No se pudo abrir el archivo\n");
	}
}


void Multiplicacion (FILE *archivo, wave *in1, wave *in2){
	int tam,i;
	wave *mayor, *menor;

	if(in1->num_samples >= in2->num_samples){
		mayor = in1;
		menor = in2;
	}
	else{
		mayor = in2;
		menor = in1;
	}

	double *aux = malloc(sizeof(double)*mayor->num_samples);
	for(i=0; i < mayor->num_samples; i++){
		aux[i] = mayor->muestras[i] * menor->muestras[i];
	}

  	Escalamiento(aux, mayor);
  	escribirArchivo(archivo, mayor);
}


double BuscaMuestraMayor(double *w, wave *signa){
	int i;
	double max = 1;
	for(i=0; i < signa->num_samples; i++)
		if(fabs(w[i]) > max)	
			max = w[i];
	return max/32767;
}

void Escalamiento(double *w, wave *entrada){
	double max = BuscaMuestraMayor(w, entrada);
	printf("\n\n\n MAx: %lf\n\n\n", max);
   	int i;
   	//printf("Samples: %lu\n",entrada->num_samples);
   	for(i=0; i < entrada->num_samples; i++){
  		entrada->muestras[i] = w[i]/max;
  		//printf("%u\n", sample[i]);
  		//if(entrada->muestras[i] > 32767)
  		//	entrada->muestras[i] = 32767;
  		//else if(entrada->muestras[i] < -32767)
  		//	entrada->muestras[i] = -32767;
  		//printf("%04x\n", entrada->muestras[i]);
   	}
}


void CrearWaveFormatMono2Stereo(short *data, wave *old, wave *header, int num_channels, long num_samples, int bits_per_sample, long sample_rate){
 	header->riff[0] = old->riff[0];
 	header->riff[1] = old->riff[1];
 	header->riff[2] = old->riff[2];
 	header->riff[3] = old->riff[3];
 	header->wave[0] = old->wave[0];
 	header->wave[1] = old->wave[1];
 	header->wave[2] = old->wave[2];
 	header->wave[3] = old->wave[3];
 	header->fmt_chunk_marker[0] = old->fmt_chunk_marker[0];
 	header->fmt_chunk_marker[1] = old->fmt_chunk_marker[1];
 	header->fmt_chunk_marker[2] = old->fmt_chunk_marker[2];
 	header->fmt_chunk_marker[3] = old->fmt_chunk_marker[3];
 	header->data_chunk_header[0] = old->data_chunk_header[0];
 	header->data_chunk_header[1] = old->data_chunk_header[1];
 	header->data_chunk_header[2] = old->data_chunk_header[2];
 	header->data_chunk_header[3] = old->data_chunk_header[3];

 	header->length_of_fmt = old->length_of_fmt;
 	//	BigEndianLong2LittleEndianChar(header->length_of_fmtb, header->length_of_fmt);
 	header->length_of_fmtb[0] = old->length_of_fmtb[0];
 	header->length_of_fmtb[1] = old->length_of_fmtb[1];
 	header->length_of_fmtb[2] = old->length_of_fmtb[2];
 	header->length_of_fmtb[3] = old->length_of_fmtb[3];
 	header->format_type = old->format_type;
 	BigEndianLong2LittleEndianChar(header->format_typeb, header->format_type, 2); 
 	header->bits_per_sample = old->bits_per_sample;
 	BigEndianLong2LittleEndianChar(header->bits_per_sampleb, header->bits_per_sample, 2);
 	//->bits_per_sampleb[0]=3;
 	//header->bits_per_sampleb[1]=4;
 	header->size_of_each_sample = 2*old->size_of_each_sample;
 	///
 	header->channels = 2;
 	BigEndianLong2LittleEndianChar(header->channelsb, header->channels, 2);
 	header->sample_rate = sample_rate;
	BigEndianLong2LittleEndianChar(header->sample_rateb, sample_rate, 4);
 	header->byterate = sample_rate * num_channels * bits_per_sample/8;
 	BigEndianLong2LittleEndianChar(header->byterateb, header->byterate, 4);
 	header->block_align = num_channels * bits_per_sample/8;
 	BigEndianLong2LittleEndianChar(header->block_alignb, header->block_align, 2);
 	header->data_size = num_samples * num_channels * bits_per_sample/8;
 	BigEndianLong2LittleEndianChar(header->data_sizeb, header->data_size, 4);
 	header->overall_size = 4+(8+16)+(8+header->data_size)+74; //74=endfile*.. 
 	printf("\n\n%ld\n", header->overall_size);
 	BigEndianLong2LittleEndianChar(header->overall_sizeb, header->overall_size, 4);
 	header->muestras = data;
 	//header->num_samples = sizeof(data)/sizeof(data[0]);
 	header->num_samples = num_samples; //si}?
 	int i; 
 	for (i=0; i<74; i++)
 		header->end[i] = old->end[i];
}

void BigEndianLong2LittleEndianChar(unsigned char *a, unsigned long in, int tam){
	if(tam == 2){
		//printf("%lX\n", in);
		a[1] = (char)(in>>8);
		a[0] = (char)(in); ///checar si se puede como abajo, en caso de ser valido
		//printf("\n\n%d %d\n", sizeof(a), sizeof(a[0])); 
	}else if(tam == 4){
		//printf("%lX\n", in);
		a[3] = ((in>>24));  // move byte 3 to byte 0
        a[2] = ((in<<8)>>24);// move byte 1 to byte 2
        a[1] = ((in<<16)>>24); // move byte 2 to byte 1
        a[0] = ((in<<24)>>24); // byte 0 to byte 3
		//printf("%x %x %x %x\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);
	}else 

	printf("no concuerda el tamaño con el formato a convertir: %s\n", a);
}


//PASAR ENTRADA A LAS FUNCIONES ES SOLO PARA CONOCER NUM_SAMPLES


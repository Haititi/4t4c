/********************************************************************
*                                                                  *
* THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
* USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
* GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
* IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
*                                                                  *
* THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2007             *
* by the Xiph.Org Foundation http://www.xiph.org/                  *
*                                                                  *
********************************************************************

function: channel mapping 0 implementation
last mod: $Id: mapping0.c 16037 2009-05-26 21:10:58Z xiphmont $

********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ogg.h>
#include "codec.h"
#include "codec_internal.h"
#include "codebook.h"
#include "window.h"
#include "registry.h"
#include "psy.h"
#include "misc.h"

/* simplistic, wasteful way of doing this (unique lookup for each
mode/submapping); there should be a central repository for
identical lookups.  That will require minor work, so I'm putting it
off as low priority.

Why a lookup for each backend in a given mode?  Because the
blocksize is set by the mode, and low backend lookups may require
parameters from other areas of the mode/mapping */

static void mapping0_free_info(vorbis_info_mapping *i){
	vorbis_info_mapping0 *info=(vorbis_info_mapping0 *)i;
	if(info){
		memset(info,0,sizeof(*info));
		_ogg_free(info);
	}
}

static int ilog(unsigned int v){
	int ret=0;
	if(v)--v;
	while(v){
		ret++;
		v>>=1;
	}
	return(ret);
}

static void mapping0_pack(vorbis_info *vi,vorbis_info_mapping *vm,oggpack_buffer *opb)
{
	int i;
	vorbis_info_mapping0 *info=(vorbis_info_mapping0 *)vm;

	/* another 'we meant to do it this way' hack...  up to beta 4, we
	packed 4 binary zeros here to signify one submapping in use.  We
	now redefine that to mean four bitflags that indicate use of
	deeper features; bit0:submappings, bit1:coupling,
	bit2,3:reserved. This is backward compatable with all actual uses
	of the beta code. */

	if(info->submaps>1){
		oggpack_write(opb,1,1);
		oggpack_write(opb,info->submaps-1,4);
	}else
		oggpack_write(opb,0,1);

	if(info->coupling_steps>0){
		oggpack_write(opb,1,1);
		oggpack_write(opb,info->coupling_steps-1,8);

		for(i=0;i<info->coupling_steps;i++){
			oggpack_write(opb,info->coupling_mag[i],ilog(vi->channels));
			oggpack_write(opb,info->coupling_ang[i],ilog(vi->channels));
		}
	}else
		oggpack_write(opb,0,1);

	oggpack_write(opb,0,2); /* 2,3:reserved */

	/* we don't write the channel submappings if we only have one... */
	if(info->submaps>1){
		for(i=0;i<vi->channels;i++)
			oggpack_write(opb,info->chmuxlist[i],4);
	}
	for(i=0;i<info->submaps;i++){
		oggpack_write(opb,0,8); /* time submap unused */
		oggpack_write(opb,info->floorsubmap[i],8);
		oggpack_write(opb,info->residuesubmap[i],8);
	}
}

/* also responsible for range checking */
static vorbis_info_mapping *mapping0_unpack(vorbis_info *vi,oggpack_buffer *opb){
	int i;
	vorbis_info_mapping0 *info=_ogg_calloc(1,sizeof(*info));
	codec_setup_info     *ci=vi->codec_setup;
	memset(info,0,sizeof(*info));

	if(oggpack_read(opb,1))
		info->submaps=oggpack_read(opb,4)+1;
	else
		info->submaps=1;

	if(oggpack_read(opb,1)){
		info->coupling_steps=oggpack_read(opb,8)+1;

		for(i=0;i<info->coupling_steps;i++){
			int testM=info->coupling_mag[i]=oggpack_read(opb,ilog(vi->channels));
			int testA=info->coupling_ang[i]=oggpack_read(opb,ilog(vi->channels));

			if(testM<0 || 
				testA<0 || 
				testM==testA || 
				testM>=vi->channels ||
				testA>=vi->channels) goto err_out;
		}

	}

	if(oggpack_read(opb,2)>0)goto err_out; /* 2,3:reserved */

	if(info->submaps>1){
		for(i=0;i<vi->channels;i++){
			info->chmuxlist[i]=oggpack_read(opb,4);
			if(info->chmuxlist[i]>=info->submaps)goto err_out;
		}
	}
	for(i=0;i<info->submaps;i++){
		oggpack_read(opb,8); /* time submap unused */
		info->floorsubmap[i]=oggpack_read(opb,8);
		if(info->floorsubmap[i]>=ci->floors)goto err_out;
		info->residuesubmap[i]=oggpack_read(opb,8);
		if(info->residuesubmap[i]>=ci->residues)goto err_out;
	}

	return info;

err_out:
	mapping0_free_info(info);
	return(NULL);
}

#include "os.h"
#include "lpc.h"
#include "lsp.h"
#include "envelope.h"
#include "mdct.h"
#include "psy.h"
#include "scales.h"

#if 0
static long seq=0;
static ogg_int64_t total=0;
static float FLOOR1_fromdB_LOOKUP[256]={
	1.0649863e-07F, 1.1341951e-07F, 1.2079015e-07F, 1.2863978e-07F, 
	1.3699951e-07F, 1.4590251e-07F, 1.5538408e-07F, 1.6548181e-07F, 
	1.7623575e-07F, 1.8768855e-07F, 1.9988561e-07F, 2.128753e-07F, 
	2.2670913e-07F, 2.4144197e-07F, 2.5713223e-07F, 2.7384213e-07F, 
	2.9163793e-07F, 3.1059021e-07F, 3.3077411e-07F, 3.5226968e-07F, 
	3.7516214e-07F, 3.9954229e-07F, 4.2550680e-07F, 4.5315863e-07F, 
	4.8260743e-07F, 5.1396998e-07F, 5.4737065e-07F, 5.8294187e-07F, 
	6.2082472e-07F, 6.6116941e-07F, 7.0413592e-07F, 7.4989464e-07F, 
	7.9862701e-07F, 8.5052630e-07F, 9.0579828e-07F, 9.6466216e-07F, 
	1.0273513e-06F, 1.0941144e-06F, 1.1652161e-06F, 1.2409384e-06F, 
	1.3215816e-06F, 1.4074654e-06F, 1.4989305e-06F, 1.5963394e-06F, 
	1.7000785e-06F, 1.8105592e-06F, 1.9282195e-06F, 2.0535261e-06F, 
	2.1869758e-06F, 2.3290978e-06F, 2.4804557e-06F, 2.6416497e-06F, 
	2.8133190e-06F, 2.9961443e-06F, 3.1908506e-06F, 3.3982101e-06F, 
	3.6190449e-06F, 3.8542308e-06F, 4.1047004e-06F, 4.3714470e-06F, 
	4.6555282e-06F, 4.9580707e-06F, 5.2802740e-06F, 5.6234160e-06F, 
	5.9888572e-06F, 6.3780469e-06F, 6.7925283e-06F, 7.2339451e-06F, 
	7.7040476e-06F, 8.2047000e-06F, 8.7378876e-06F, 9.3057248e-06F, 
	9.9104632e-06F, 1.0554501e-05F, 1.1240392e-05F, 1.1970856e-05F, 
	1.2748789e-05F, 1.3577278e-05F, 1.4459606e-05F, 1.5399272e-05F, 
	1.6400004e-05F, 1.7465768e-05F, 1.8600792e-05F, 1.9809576e-05F, 
	2.1096914e-05F, 2.2467911e-05F, 2.3928002e-05F, 2.5482978e-05F, 
	2.7139006e-05F, 2.8902651e-05F, 3.0780908e-05F, 3.2781225e-05F, 
	3.4911534e-05F, 3.7180282e-05F, 3.9596466e-05F, 4.2169667e-05F, 
	4.4910090e-05F, 4.7828601e-05F, 5.0936773e-05F, 5.4246931e-05F, 
	5.7772202e-05F, 6.1526565e-05F, 6.5524908e-05F, 6.9783085e-05F, 
	7.4317983e-05F, 7.9147585e-05F, 8.4291040e-05F, 8.9768747e-05F, 
	9.5602426e-05F, 0.00010181521F, 0.00010843174F, 0.00011547824F, 
	0.00012298267F, 0.00013097477F, 0.00013948625F, 0.00014855085F, 
	0.00015820453F, 0.00016848555F, 0.00017943469F, 0.00019109536F, 
	0.00020351382F, 0.00021673929F, 0.00023082423F, 0.00024582449F, 
	0.00026179955F, 0.00027881276F, 0.00029693158F, 0.00031622787F, 
	0.00033677814F, 0.00035866388F, 0.00038197188F, 0.00040679456F, 
	0.00043323036F, 0.00046138411F, 0.00049136745F, 0.00052329927F, 
	0.00055730621F, 0.00059352311F, 0.00063209358F, 0.00067317058F, 
	0.00071691700F, 0.00076350630F, 0.00081312324F, 0.00086596457F, 
	0.00092223983F, 0.00098217216F, 0.0010459992F, 0.0011139742F, 
	0.0011863665F, 0.0012634633F, 0.0013455702F, 0.0014330129F, 
	0.0015261382F, 0.0016253153F, 0.0017309374F, 0.0018434235F, 
	0.0019632195F, 0.0020908006F, 0.0022266726F, 0.0023713743F, 
	0.0025254795F, 0.0026895994F, 0.0028643847F, 0.0030505286F, 
	0.0032487691F, 0.0034598925F, 0.0036847358F, 0.0039241906F, 
	0.0041792066F, 0.0044507950F, 0.0047400328F, 0.0050480668F, 
	0.0053761186F, 0.0057254891F, 0.0060975636F, 0.0064938176F, 
	0.0069158225F, 0.0073652516F, 0.0078438871F, 0.0083536271F, 
	0.0088964928F, 0.009474637F, 0.010090352F, 0.010746080F, 
	0.011444421F, 0.012188144F, 0.012980198F, 0.013823725F, 
	0.014722068F, 0.015678791F, 0.016697687F, 0.017782797F, 
	0.018938423F, 0.020169149F, 0.021479854F, 0.022875735F, 
	0.024362330F, 0.025945531F, 0.027631618F, 0.029427276F, 
	0.031339626F, 0.033376252F, 0.035545228F, 0.037855157F, 
	0.040315199F, 0.042935108F, 0.045725273F, 0.048696758F, 
	0.051861348F, 0.055231591F, 0.058820850F, 0.062643361F, 
	0.066714279F, 0.071049749F, 0.075666962F, 0.080584227F, 
	0.085821044F, 0.091398179F, 0.097337747F, 0.10366330F, 
	0.11039993F, 0.11757434F, 0.12521498F, 0.13335215F, 
	0.14201813F, 0.15124727F, 0.16107617F, 0.17154380F, 
	0.18269168F, 0.19456402F, 0.20720788F, 0.22067342F, 
	0.23501402F, 0.25028656F, 0.26655159F, 0.28387361F, 
	0.30232132F, 0.32196786F, 0.34289114F, 0.36517414F, 
	0.38890521F, 0.41417847F, 0.44109412F, 0.46975890F, 
	0.50028648F, 0.53279791F, 0.56742212F, 0.60429640F, 
	0.64356699F, 0.68538959F, 0.72993007F, 0.77736504F, 
	0.82788260F, 0.88168307F, 0.9389798F, 1.F, 
};

#endif 


static int mapping0_inverse(vorbis_block *vb,vorbis_info_mapping *l){
	vorbis_dsp_state     *vd=vb->vd;
	vorbis_info          *vi=vd->vi;
	codec_setup_info     *ci=vi->codec_setup;
	private_state        *b=vd->backend_state;
	vorbis_info_mapping0 *info=(vorbis_info_mapping0 *)l;

	int                   i,j;
	long                  n=vb->pcmend=ci->blocksizes[vb->W];

	float **pcmbundle=_alloca(sizeof(*pcmbundle)*vi->channels);
	int    *zerobundle=_alloca(sizeof(*zerobundle)*vi->channels);

	int   *nonzero  =_alloca(sizeof(*nonzero)*vi->channels);
	void **floormemo=_alloca(sizeof(*floormemo)*vi->channels);

	/* recover the spectral envelope; store it in the PCM vector for now */
	for(i=0;i<vi->channels;i++)
	{
		int submap=info->chmuxlist[i];
		floormemo[i]=_floor_P[ci->floor_type[info->floorsubmap[submap]]]->
			inverse1(vb,b->flr[info->floorsubmap[submap]]);
		if(floormemo[i])
			nonzero[i]=1;
		else
			nonzero[i]=0;      
		memset(vb->pcm[i],0,sizeof(*vb->pcm[i])*n/2);
	}

	/* channel coupling can 'dirty' the nonzero listing */
	for(i=0;i<info->coupling_steps;i++)
	{
		if(nonzero[info->coupling_mag[i]] || nonzero[info->coupling_ang[i]])
		{
			nonzero[info->coupling_mag[i]]=1; 
			nonzero[info->coupling_ang[i]]=1; 
		}
	}

	/* recover the residue into our working vectors */
	for(i=0;i<info->submaps;i++)
	{
		int ch_in_bundle=0;
		for(j=0;j<vi->channels;j++)
		{
			if(info->chmuxlist[j]==i)
			{
				if(nonzero[j])
					zerobundle[ch_in_bundle]=1;
				else
					zerobundle[ch_in_bundle]=0;
				pcmbundle[ch_in_bundle++]=vb->pcm[j];
			}
		}

		_residue_P[ci->residue_type[info->residuesubmap[i]]]->
			inverse(vb,b->residue[info->residuesubmap[i]],pcmbundle,zerobundle,ch_in_bundle);
	}

	/* channel coupling */
	for(i=info->coupling_steps-1;i>=0;i--)
	{
		float *pcmM=vb->pcm[info->coupling_mag[i]];
		float *pcmA=vb->pcm[info->coupling_ang[i]];

		for(j=0;j<n/2;j++)
		{
			float mag=pcmM[j];
			float ang=pcmA[j];

			if(mag>0)
				if(ang>0){
					pcmM[j]=mag;
					pcmA[j]=mag-ang;
				}else{
					pcmA[j]=mag;
					pcmM[j]=mag+ang;
				}
			else
				if(ang>0){
					pcmM[j]=mag;
					pcmA[j]=mag+ang;
				}else{
					pcmA[j]=mag;
					pcmM[j]=mag-ang;
				}
		}
	}

	/* compute and apply spectral envelope */
	for(i=0;i<vi->channels;i++)
	{
		float *pcm=vb->pcm[i];
		int submap=info->chmuxlist[i];
		_floor_P[ci->floor_type[info->floorsubmap[submap]]]->
			inverse2(vb,b->flr[info->floorsubmap[submap]],floormemo[i],pcm);
	}

	/* transform the PCM data; takes PCM vector, vb; modifies PCM vector */
	/* only MDCT right now.... */
	for(i=0;i<vi->channels;i++)
	{
		float *pcm=vb->pcm[i];
		mdct_backward(b->transform[vb->W][0],pcm,pcm);
	}

	/* all done! */
	return(0);
}

/* export hooks */
const vorbis_func_mapping mapping0_exportbundle={
	&mapping0_pack,
	&mapping0_unpack,
	&mapping0_free_info,
	NULL,
	&mapping0_inverse
};


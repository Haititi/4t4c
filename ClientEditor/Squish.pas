unit Squish;

interface
//delphi header by Chaotikmind 2009
  type u8=byte;
       Pu8=^u8;

  const SquishDll='squish.dll';

	//! Use DXT1 compression.
	const kDxt1:cardinal= ( 1 shl 0 );
	//! Use DXT3 compression.
	const kDxt3:cardinal = ( 1 shl 1 );
	//! Use DXT5 compression.
	const kDxt5:cardinal = ( 1 shl 2 );
	//! Use a very slow but very high quality colour compressor.
	const kColourIterativeClusterFit:cardinal = ( 1 shl 8 );
	//! Use a slow but high quality colour compressor (the default).
	const kColourClusterFit:cardinal = ( 1 shl 3 );
	//! Use a fast but low quality colour compressor.
	const kColourRangeFit:cardinal	= ( 1 shl 4 );
	//! Use a perceptual metric for colour error (the default).
	const kColourMetricPerceptual:cardinal = ( 1 shl 5 );
	//! Use a uniform metric for colour error.
	const kColourMetricUniform:cardinal = ( 1 shl 6 );
	//! Weight the colour by alpha during cluster fit (disabled by default).
	const kWeightColourByAlpha:cardinal = ( 1 shl 7 );


  Procedure Compress(rgba:Pu8; Block:pointer; Flags:cardinal ); cdecl;
  {$EXTERNALSYM Compress}
  Procedure CompressMasked(rgba:Pu8; Mask:integer; Block:integer; Flags:cardinal); cdecl;
  {$EXTERNALSYM CompressMasked}
  Procedure Decompress(rgba:Pu8; const Block:pointer; Flags:cardinal ); cdecl;
  {$EXTERNALSYM Decompress}
  Function GetStorageRequirements(Width:integer; Height:integer; Flags:cardinal ):integer; cdecl;
  {$EXTERNALSYM GetStorageRequirements}
  Procedure CompressImage(rgba:Pu8; Width:integer; Height:integer; Blocks:pointer; Flags:cardinal ); cdecl;
  {$EXTERNALSYM CompressImage}
  Procedure DecompressImage( rgba:Pu8; Width:integer; Height:integer; const Blocks:pointer; Flags:cardinal ); cdecl;
  {$EXTERNALSYM DecompressImage}

implementation

procedure Compress;               external SquishDll index 1;// 'Compress';
procedure CompressMasked;         external SquishDll index 3;//name 'CompressMasked';
procedure Decompress;             external SquishDll index 4;//name 'Decompress';
function  GetStorageRequirements; external SquishDll index 6;//name 'GetStorageRequirements';
procedure CompressImage;          external SquishDll index 2;//name 'CompressImage';
procedure DecompressImage;        external SquishDll index 5;//name 'DecompressImage';

end.

unit IdTypes;

interface
uses classes;

 type

   TGraphInfo=record
    GraphName:String;
    VertexCol:Cardinal;
    Offx,Offy,Offz:smallint;
   end;
   PGraphInfo=^TGraphInfo;

  TMapIdInfo=record
   Defined:Cardinal;
   IdType:Cardinal;
   Reversed:Cardinal;
   Modx,Mody:longint;
   MinimapColor:Cardinal;
   MiniMapTexPx,
   MiniMapTexPy,
   MiniMapTexPx2,
   MiniMapTexPy2:single; //minimapTexturing coordinate
   LightInfo:Cardinal; //0:no shadow 2:need to draw dynamic shadow 4:this object cast light 8:this object block light
   ColorFx:Cardinal;
   GraphCount:Cardinal;
   GraphInfo:TList;      //list of TGraphInfo
  end;

  TSoundInfo=record
   SoundName:string;
   PitchDev:single;
  end;
  PSoundInfo=^TSoundInfo;

  TFrame=record
   SpriteName:string;
   Offx,Offy:smallint;
  end;
  PFrame=^TFrame;

  TFrameList=record
   Reversed:Cardinal;
   GraphCount:Cardinal;
   FrameList:TList; //list of TFrame
  end;

  TDirectionAnim=array[0..7]of TFrameList;

  TMonsterSkinInfo=record
   SkinName:string;
   SkinId:word;
   VertexColor:cardinal;
   ColorFx:Cardinal;
   AnimationDelay:single;  //in seconds
   MonsterSize:single;  //shrinkening or growing factor   1 = normal size
   Walk,Attack:TDirectionAnim;
   Death:TFrameList;
   AtkSoundCount:cardinal;
   AtkSounds:TList; //list of TSoundInfo
   HitSoundCount:cardinal;
   HitSounds:TList; //list of TSoundInfo
   DieSoundCount:cardinal;
   DieSounds:TList; //list of TSoundInfo
   IdleSoundCount:cardinal;
   IdleSounds:TList; //list of TSoundInfo
  end;
  PMonsterSkinInfo=^TMonsterSkinInfo;


  //NOTE : We could use the previous structure for puppet also since it's very very similar
  TPuppetSkinPartInfo=record
   SkinName:string;
   SkinId:word;              //it is the Appearance in fact (same for male and female)
                            //Last bit is set to 1 for a female puppet appearance
   Female:boolean;
   VertexColor:cardinal;
   ColorFx:Cardinal;
   AnimationDelay:single;
   Walk,Attack,Range:TDirectionAnim;
   Death:TFrameList;
  end;


  PPuppetSkinPartInfo=^TPuppetSkinPartInfo;



  TItemSkinInfo=record
                 SkinName:string;
                 SkinId:word;
                 SkinType:Cardinal;
                 MouseCursor:Cardinal;
                 ColorFx:cardinal;
                 VertexColor:cardinal;
                 AnimationDelay:single;
                 OpenSound:string;
                 CloseSound:string;
                 GraphCount:Cardinal;
                 GraphInfo:TList;       //list of TFrame
                end;
  PItemSkinInfo=^TItemSkinInfo;


implementation

end.

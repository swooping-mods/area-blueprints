#define MOD_NAME "Area Blueprints"
#include "common.h"
#include "../build-menu-framework/sdk/construction_unity.h"
#include "../../runtime/native/camera.h"
#include "../../runtime/native/input.h"
#include "input.h"
#include "storage.h"
#include "hud.h"
#include "shop_signal.h"
#define BP_MAX 128
/* Original fixed-width format. No object identities, resources or save bytes. */
typedef struct {int kind,foundation;DVec offset;float yaw;} Piece;
typedef struct {DWORD magic,version,count,reserved;DVec anchor;Piece pieces[BP_MAX];DWORD checksum;} Blueprint;
static Blueprint bp;
static const MoonCameraApiV1 *survey_api;static MoonCameraSession survey_session;
static DRoot survey_walker;
static struct {
 PTR flying,set_flying,teleport,depots,pose,yaw,definition,is_foundation;
 MoonBuildPlacementApi construction;MoonBuildPlacement placement;
 PTR advance,done,check_valid,tool,free_record;
 DVec a,b,return_pos,destination;int editor,leaving,corner,was_flying,building,index,slot;
 char path[240];const char *status;
} bs;
static DWORD hash(const void *p,U64 n){const unsigned char *s=p;DWORD h=2166136261u;while(n--)h=(h^*s++)*16777619u;return h;}
static PTR builder(void){int ok=1;void *a[]={dtype("MoonCleaners","MoonDepotBuilder")};return call(cm.component,dget(cm.walker),a,&ok);}
static void path(void){char *p=cat(bs.path,io.base);p=cat(p,"\\slot");p=num(p,(U64)bs.slot);cat(p,".mbp");}
static void cancel(void){bs.building=0;moon_build_placement_cancel(&bs.placement);}
#include "camera_restore.h"
static int capture_survey(void *context){PTR walker=context;int ok=1;bs.return_pos=dvector(cm.position,ctr(walker,&ok),&ok);bs.was_flying=boolean(bs.flying,walker,&ok);if(ok)survey_walker=dhold(walker);return ok&&survey_walker;}

static void capture(void){int ok=1;static Blueprint candidate;dzero(&candidate,sizeof(candidate));candidate.magic=0x50424c4d;candidate.version=1;candidate.anchor=bs.a;
 DRoot collection=dhold(call(bs.depots,dget(cm.world),0,&ok));PTR items=dget(collection);DRoot it=dhold(items?call(get_method(object_class(items),"GetEnumerator",0),items,0,&ok):0);PTR boxed=dget(it),klass=boxed?object_class(boxed):0;PTR self=boxed?(cm.is_value(klass)?unbox(boxed):boxed):0;int overflow=0,unsupported=0;
 for(int i=0;self&&i<20000&&boolean(get_method(klass,"MoveNext",0),self,&ok);i++){
  DRoot record=dhold(call(get_method(klass,"get_Current",0),self,0,&ok));PTR r=dget(record);if(!r){ddrop(&record);break;}DVec p={0};Quat q={0};void *pose[]={r,&p,&q};call(bs.pose,dget(cm.world),pose,&ok);
  /* The selection is a world-aligned volume padded vertically on all axes;
   * small lunar sites remain useful regardless of their radial orientation. */
  if(valid(p)&&moon_build_in_box((MoonBuildPoint){p.x,p.y,p.z},(MoonBuildPoint){bs.a.x,bs.a.y,bs.a.z},(MoonBuildPoint){bs.b.x,bs.b.y,bs.b.z},2)){
   void *free_args[]={r};PTR free_value=call(bs.free_record,dget(cm.world),free_args,&ok);if(!ok){ddrop(&record);break;}if(free_value&&*(unsigned char*)unbox(free_value)){unsupported++;ddrop(&record);continue;}
   if(candidate.count>=BP_MAX){overflow=1;ddrop(&record);break;}
   Piece *piece=candidate.pieces+candidate.count++;piece->kind=ci(r,"kind");void *kind[]={&piece->kind};PTR yes=call(bs.is_foundation,dget(cm.world),kind,&ok);piece->foundation=yes&&*(unsigned char*)unbox(yes);piece->offset=ca(p,cx(bs.a,-1));void *yaw[]={r};PTR y=call(bs.yaw,dget(cm.world),yaw,&ok);piece->yaw=y?*(float*)unbox(y):0;
  }ddrop(&record);if(!ok)break;
 }
 PTR dispose=klass?get_method(klass,"Dispose",0):0;if(dispose)call(dispose,self,0,&ok);ddrop(&it);ddrop(&collection);
 if(!ok||overflow||!candidate.count){bs.status=overflow?"Area too large: maximum 128 pieces":"Selection empty or unavailable";return;}candidate.checksum=hash(&candidate,sizeof(candidate)-4);if(!iowrite(bs.path,&candidate,sizeof(candidate))){bs.status="Blueprint file could not be saved";return;}dcopy(&bp,&candidate,sizeof(bp));bs.status=unsupported?"Saved standard buildings; free-pose pieces excluded":"Blueprint saved";sx_event("blueprint-saved",(int)bp.count);
}
static int load(void){static Blueprint candidate;dzero(&candidate,sizeof(candidate));if(!ioread(bs.path,&candidate,sizeof(candidate))||candidate.magic!=0x50424c4d||candidate.version!=1||!candidate.count||candidate.count>BP_MAX||candidate.checksum!=hash(&candidate,sizeof(candidate)-4)||!valid(candidate.anchor))return 0;for(DWORD i=0;i<candidate.count;i++){Piece *p=candidate.pieces+i;if(p->kind<0||p->kind>100000||p->foundation<0||p->foundation>1||!valid(p->offset)||!finite(p->yaw)||cd(p->offset,p->offset)>40000)return 0;}dcopy(&bp,&candidate,sizeof(bp));return 1;}
static void step(float dt){
 if(!bs.building||bs.index>=(int)bp.count){cancel();return;}
 Piece *piece=bp.pieces+bs.index;DVec destination=ca(bs.destination,piece->offset);MoonBuildPoint point={destination.x,destination.y,destination.z};
 if(!bs.placement.check&&!moon_build_placement_begin(&bs.placement,&bs.construction,dget(cm.world),piece->kind,piece->foundation,point,piece->yaw)){cancel();bs.status="Placement planner rejected piece; stopped";return;}
 int state=moon_build_placement_advance(&bs.placement,dt,20,1.0);
 if(state==MOON_BUILD_WAIT)return;
 if(state!=MOON_BUILD_READY){cancel();bs.status=state==MOON_BUILD_SIGNATURE?"Placement Advance signature changed; stopped":state==MOON_BUILD_INVALID?"Site invalid: stopped; placed pieces remain":"Placement check unavailable or timed out";return;}
 if(!moon_build_placement_commit(&bs.placement,&bs.construction,dget(cm.world),dget(cm.walker))){cancel();bs.status="Build rejected (cost, unlock or site); stopped";return;}
 bs.index++;moon_build_placement_cancel(&bs.placement);sx_event("blueprint-piece-built",bs.index);
 if(bs.index>=(int)bp.count){cancel();bs.status="Blueprint rebuilt; vanilla costs paid";}
}
/* Camera claims arbitrate against the current Update, including a drone
 * deployed earlier in the same frame. Player kinematic state is not a claim. */
static int before(int id,Context *c){(void)id;PTR walker=(PTR)c->rcx;if(!sx_thread())return 0;if(bs.leaving){leave();if(survey_session.token){(void)blueprint_input(0);return 0;}}int state=cframe(walker);int allowed=state>0&&cinput(walker);BlueprintInput input=blueprint_input(allowed);if(state<0){leave();if(survey_session.token)return 0;ddrop(&cm.world);ddrop(&cm.walker);int ok=1;cm.world=dhold(sx_world(walker,&ok));cm.walker=dhold(walker);return 0;}if(state<=0)return 0;int requested=input.survey.pressed;if(requested)sx_event("survey-request",0);if(!allowed){if(requested){int gate_ok=1;int reason=!sx_input(walker,&gate_ok)|(!boolean(cm.first,walker,&gate_ok)<<1)|(boolean(cm.menu,walker,&gate_ok)<<2)|(!sx_host(&gate_ok)<<3)|(!gate_ok<<4);sx_event("survey-input-refused",reason);}if(bs.editor)leave();return 0;}int ok=1;
 if(moon_shop_poll()||requested){if(bs.editor)leave();else if(!moon_camera_begin(&survey_session,survey_api,"area-blueprints",dget(cm.world),walker,cf(walker,"viewCamera"),MOON_CAMERA_POSE|MOON_CAMERA_MODE,capture_survey,walker)){sx_event("survey-camera-busy",0);bs.status="Return the remote camera before survey flight";}else{unsigned char yes=1,no=0;void *a[]={&yes,&no};call(bs.set_flying,walker,a,&ok);bs.editor=1;if(!ok)leave();sx_event(ok?"survey-entered":"survey-enter-failed",0);bs.status="Select opposite corners with F6 / F7";load();}}

 if(!bs.editor)return 0;
 if(input.leave.pressed){leave();return 0;}
 if(input.first.pressed&&!bs.building){if(craycast(&bs.a)){bs.corner=1;bs.status="First corner set";}else bs.status="Aim at loaded ground or a building";}
 if(input.second.pressed&&bs.corner&&!bs.building){if(craycast(&bs.b)){capture();bs.corner=0;}else bs.status="No ground under reticle";}
 if(input.slot.pressed&&!bs.building){bs.slot=bs.slot%5+1;path();dzero(&bp,sizeof(bp));bs.status=load()?"Blueprint slot loaded":"Empty blueprint slot";}
 if(input.rebuild.pressed&&!bs.building){PTR b=builder();if(!b||!boolean(bs.tool,b,&ok))bs.status="Hold the normal building tool to rebuild";else if(!bp.count)bs.status="Save or select a blueprint first";else if(craycast(&bs.destination)){bs.building=1;bs.index=0;bs.status="Building one paid piece at a time; Esc cancels";}}
 float dt=dnumber(cm.delta,0,&ok);if(bs.building&&ok&&dt>0&&dt<.2f)step(dt);
 char survey[64]="Alt+B",back[64]="Escape",first[64]="F6",second[64]="F7",slot[64]="F8",rebuild[64]="Ctrl+R";
 moon_input_label("area-blueprints.survey","Alt+B",survey,sizeof(survey));moon_input_label("area-blueprints.leave","Escape",back,sizeof(back));moon_input_label("area-blueprints.first-corner","F6",first,sizeof(first));moon_input_label("area-blueprints.save-corner","F7",second,sizeof(second));moon_input_label("area-blueprints.next-slot","F8",slot,sizeof(slot));moon_input_label("area-blueprints.rebuild","Ctrl+R",rebuild,sizeof(rebuild));
 char text[1200],*p=cat(text,"Area Blueprints | ");p=cat(p,survey);p=cat(p," / ");p=cat(p,back);p=cat(p," return | Flight: normal game controls\n");p=cat(p,first);p=cat(p," first corner / ");p=cat(p,second);p=cat(p," save second | ");p=cat(p,slot);p=cat(p," slot ");p=num(p,(U64)bs.slot);p=cat(p," | ");p=cat(p,rebuild);p=cat(p," rebuild at reticle\n");p=num(p,bp.count);p=cat(p," pieces | built ");p=num(p,(U64)bs.index);p=cat(p,"\n");cat(p,bs.status?bs.status:"");hshow(text);return 0;}
__declspec(dllexport) int MoonLab_ModStart(PTR domain,PTR game){if(!sx_start(domain,game,"area-blueprints","MOONLAB_BLUEPRINTS_CONFIG")||!cbind(game)||!hbind()||!iobind("area-blueprints"))return 0;
#define B(F,T,M,S) bs.F=dmethod("MoonCleaners",T,M,S);if(!bs.F){sx_event(T "::" M,0);return 0;}
 B(flying,"MoonWalker","get_Flying","");B(set_flying,"MoonWalker","SetFlying","System.Boolean;System.Boolean");B(teleport,"MoonWalker","Teleport","UnityEngine.Vector3;System.Boolean");B(depots,"MoonWorld","get_BuiltDepots","");B(pose,"MoonWorld","StructurePose","MoonCleaners.MoonDepotSnapshot;UnityEngine.Vector3&;UnityEngine.Quaternion&");B(yaw,"MoonWorld","BuildYawOf","MoonCleaners.MoonDepotSnapshot");B(is_foundation,"MoonWorld","IsFoundation","System.Int32");B(free_record,"MoonWorld","FreeGridRecord","MoonCleaners.MoonDepotSnapshot");B(tool,"MoonDepotBuilder","get_HoldingBuildTool","");
#undef B
 if(!moon_build_placement_bind(&bs.construction))return 0;
 survey_api=moon_camera_connect();if(!survey_api)return 0;bs.slot=sx_limit("blueprints","slot",1,1,5);path();(void)ci;(void)cn;(void)cs;(void)ccharge;(void)unit;(void)worldkey;(void)deq;(void)dzero;(void)moon_shop_unlisten;if(!moon_shop_listen("area-blueprints"))return 0;sx_event("ready-testing-deferred",0);return sx_install(game,before);}

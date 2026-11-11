/* Each package owns this source copy and resolves all APIs independently. */
#include "../../runtime/native/gameplay.h"
#include "support.h"
typedef struct {float x,y,z,w;} Quat;
static struct {
 PTR transform,position,set_position,rotation,set_rotation,forward,right,up;
 PTR delta,frame,component,first,menu,save_root,wallet,charge,decimal_ctor,decimal_int,ray,hit_point;
 DRoot world,walker; int frame_id,down[256],press[256];
 void (*set_field)(PTR,PTR,void *);
 int (*is_value)(PTR);
} cm;
static inline PTR cf(PTR o,const char *n){PTR v=0,f=o?get_field(object_class(o),n):0;if(f)field_value(o,f,&v);return v;}
static inline int ci(PTR o,const char *n){int v=0;PTR f=o?get_field(object_class(o),n):0;if(f)field_value(o,f,&v);return v;}
static inline float cn(PTR o,const char *n){float v=0;PTR f=o?get_field(object_class(o),n):0;if(f)field_value(o,f,&v);return v;}
static inline void cs(PTR o,const char *n,void *v){PTR f=o?get_field(object_class(o),n):0;if(f)cm.set_field(o,f,v);}
static inline void cv(PTR m,PTR o,void *v,int *ok){void *a[]={v};call(m,o,a,ok);}
static inline DVec ca(DVec a,DVec b){return (DVec){a.x+b.x,a.y+b.y,a.z+b.z};}
static inline DVec cx(DVec a,float s){return (DVec){a.x*s,a.y*s,a.z*s};}
static inline float cd(DVec a,DVec b){return a.x*b.x+a.y*b.y+a.z*b.z;}
static inline float croot(float v){if(v<=0)return 0;float q=v>1?v:1;for(int i=0;i<18;i++)q=(q+v/q)*.5f;return q;}
static inline DVec unit(DVec a){float n=croot(cd(a,a));return n>.0001f?cx(a,1/n):(DVec){0,1,0};}
static inline int finite(float v){return v==v&&v<1e7f&&v>-1e7f;}
static inline int valid(DVec v){return finite(v.x)&&finite(v.y)&&finite(v.z);}
static inline PTR ctr(PTR o,int *ok){return call(cm.transform,o,0,ok);}
static inline int craycast(DVec *p){int ok=1;PTR camera=cf(dget(cm.walker),"viewCamera"),tr=ctr(camera,&ok);DVec o=dvector(cm.position,tr,&ok),d=dvector(cm.forward,tr,&ok);float distance=200;int mask=~(1<<2),trigger=1;union{U64 align;unsigned char bytes[96];}hit={0};void *a[]={&o,&d,hit.bytes,&distance,&mask,&trigger};PTR yes=call(cm.ray,0,a,&ok);if(!ok||!yes||!*(unsigned char*)unbox(yes))return 0;*p=dvector(cm.hit_point,hit.bytes,&ok);return ok&&valid(*p);}
static inline int ccharge(int cost){int ok=1;DRoot player=dhold(call(sx.player,0,0,&ok));void *a[]={dget(player)};DRoot money=dhold(call(cm.wallet,dget(cm.world),a,&ok));void *b[]={money?unbox(dget(money)):0};PTR value=b[0]?call(cm.decimal_int,0,b,&ok):0;int funds=value?*(int*)unbox(value):-1;ddrop(&money);if(!ok||funds<cost){ddrop(&player);return 0;}int negative=-cost;unsigned char decimal[16]={0};void *ctor[]={&negative};call(cm.decimal_ctor,decimal,ctor,&ok);void *debit[]={dget(player),decimal};call(cm.charge,dget(cm.world),debit,&ok);ddrop(&player);return ok;}
static inline int cframe(PTR walker){if(!sx_thread())return 0;int ok=1,f=integer(cm.frame,0,&ok);if(!ok||cm.frame_id==f)return 0;cm.frame_id=f;PTR world=sx_world(walker,&ok);for(int k=0;k<256;k++){int down=sx_down(k);cm.press[k]=down&&!cm.down[k];cm.down[k]=down;}if(world!=dget(cm.world)||walker!=dget(cm.walker)){return -1;}return world&&ok?1:0;}
static inline int cinput(PTR walker){int ok=1;return sx_input(walker,&ok)&&boolean(cm.first,walker,&ok)&&!boolean(cm.menu,walker,&ok)&&sx_host(&ok)&&ok;}
static inline int cbind(PTR game){
#define C(F,N,T,M,S) cm.F=dmethod(N,T,M,S);if(!cm.F){sx_event(T "::" M,0);return 0;}
 C(transform,"UnityEngine","Component","get_transform","");C(position,"UnityEngine","Transform","get_position","");C(set_position,"UnityEngine","Transform","set_position","UnityEngine.Vector3");C(rotation,"UnityEngine","Transform","get_rotation","");C(set_rotation,"UnityEngine","Transform","set_rotation","UnityEngine.Quaternion");C(forward,"UnityEngine","Transform","get_forward","");C(right,"UnityEngine","Transform","get_right","");C(up,"UnityEngine","Transform","get_up","");
 C(delta,"UnityEngine","Time","get_unscaledDeltaTime","");C(frame,"UnityEngine","Time","get_frameCount","");C(component,"UnityEngine","Component","GetComponent","System.Type");C(first,"MoonCleaners","MoonWalker","get_IsFirstPerson","");C(menu,"MoonCleaners","MoonWalker","get_MenuBlocked","");C(save_root,"MoonCleaners","MoonWorld","get_SaveDirectory","");C(wallet,"MoonCleaners","MoonWorld","WalletDollars","System.String");C(charge,"MoonCleaners","MoonWorld","ChargeBuild","System.String;System.Decimal");C(decimal_ctor,"System","Decimal",".ctor","System.Int32");C(decimal_int,"System","Decimal","ToInt32","System.Decimal");C(ray,"UnityEngine","Physics","Raycast","UnityEngine.Vector3;UnityEngine.Vector3;UnityEngine.RaycastHit&;System.Single;System.Int32;UnityEngine.QueryTriggerInteraction");C(hit_point,"UnityEngine","RaycastHit","get_point","");
#undef C
 cm.set_field=(void(*)(PTR,PTR,void *))GetProcAddress(game,"il2cpp_field_set_value");cm.is_value=(int(*)(PTR))GetProcAddress(game,"il2cpp_class_is_valuetype");cm.frame_id=-1;return cm.set_field&&cm.is_value;
}

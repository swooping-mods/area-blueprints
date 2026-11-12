/* Version 1 remains readable; version 2 adds tagged free-grid poses and belts.
 * No native object, source-world ID, resource or personal-save bytes are stored. */
#define BP_MAX 128
#define BP_MAGIC 0x50424c4d
enum { BP_GRID=0, BP_FREE=1, BP_BELT=2 };
typedef struct {int kind,foundation;DVec offset;float yaw;} PieceV1;
typedef struct {DWORD magic,version,count,reserved;DVec anchor;PieceV1 pieces[BP_MAX];DWORD checksum;} BlueprintV1;
typedef struct {
 int kind,foundation;DVec offset;DWORD mode;
 union {
  struct {float yaw;Quat rotation;} pose;
  struct {DVec end,start_direction,end_direction;} belt;
 } data;
} Piece;
typedef struct {DWORD magic,version,count,reserved;DVec anchor;Piece pieces[BP_MAX];DWORD checksum;} Blueprint;
_Static_assert(sizeof(BlueprintV1)==3104,"Legacy slot layout changed");
_Static_assert(sizeof(Piece)==60&&sizeof(Blueprint)==7712,"Version 2 slot layout changed");
static DWORD hash(const void *p,U64 n){const unsigned char *s=p;DWORD h=2166136261u;while(n--)h=(h^*s++)*16777619u;return h;}
static int bp_offset(DVec v){return valid(v)&&cd(v,v)<=40000;}
static int bp_direction(DVec v){float n=cd(v,v);return valid(v)&&n>.99f&&n<1.01f;}
static int bp_piece_valid(const Piece *p){
 if(p->mode>BP_BELT||p->kind<0||p->kind>100000||p->foundation<0||p->foundation>1||!bp_offset(p->offset)||(p->mode!=BP_BELT&&!finite(p->data.pose.yaw)))return 0;
 if(p->mode==BP_FREE){float n=p->data.pose.rotation.x*p->data.pose.rotation.x+p->data.pose.rotation.y*p->data.pose.rotation.y+p->data.pose.rotation.z*p->data.pose.rotation.z+p->data.pose.rotation.w*p->data.pose.rotation.w;return p->foundation&&finite(n)&&n>.99f&&n<1.01f;}
 if(p->mode==BP_BELT)return !p->kind&&!p->foundation&&bp_offset(p->data.belt.end)&&bp_direction(p->data.belt.start_direction)&&bp_direction(p->data.belt.end_direction)&&cd(ca(p->data.belt.end,cx(p->offset,-1)),ca(p->data.belt.end,cx(p->offset,-1)))>.0001f;
 return 1;
}
static int bp_valid(const Blueprint *b){
 if(b->magic!=BP_MAGIC||b->version!=2||b->reserved||!b->count||b->count>BP_MAX||!valid(b->anchor))return 0;
 for(DWORD i=0;i<b->count;i++)if(!bp_piece_valid(b->pieces+i))return 0;
 return 1;
}
static int bp_upgrade(const BlueprintV1 *old,Blueprint *out){
 if(old->magic!=BP_MAGIC||old->version!=1||old->reserved||!old->count||old->count>BP_MAX||!valid(old->anchor)||old->checksum!=hash(old,sizeof(*old)-4))return 0;
 dzero(out,sizeof(*out));out->magic=BP_MAGIC;out->version=2;out->count=old->count;out->anchor=old->anchor;
 for(DWORD i=0;i<old->count;i++){out->pieces[i].kind=old->pieces[i].kind;out->pieces[i].foundation=old->pieces[i].foundation;out->pieces[i].offset=old->pieces[i].offset;out->pieces[i].data.pose.yaw=old->pieces[i].yaw;}
 return bp_valid(out);
}

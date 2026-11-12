/* Inspected Steam metadata: PlanStructure/FreeGridRecord and the ConveyorPlan
 * overload of TryBuildConveyor. Resolve types/fields, never native addresses. */
static struct {
 PTR conveyors,build_belt,belt_class,belt_ctor,free_pose,depot_pose;
 PTR start,end,start_direction,end_direction;
 PTR belt_fields[11],plan_position,plan_rotation,plan_free,plan_module,support;
} ba;
static int bp_bool(PTR method,PTR self,void **args,int *ok){
 if(!*ok)return 0;
 PTR result=call(method,self,args,ok);unsigned char *value=*ok&&result?unbox(result):0;
 if(!*ok||!value){*ok=0;return 0;}return *value!=0;
}
static PTR bp_field(PTR klass,const char *name,const char *expected,PTR (*field_type)(PTR)){
 PTR f=klass?get_field(klass,name):0,raw=f?field_type(f):0;char *type=raw?da.type_name(raw):0;
 int match=type&&deq(type,expected);if(type)da.free(type);return match?f:0;
}
static int bp_bind(PTR game){
 PTR (*from_type)(PTR)=(PTR(*)(PTR))GetProcAddress(game,"il2cpp_class_from_type");
 PTR (*field_type)(PTR)=(PTR(*)(PTR))GetProcAddress(game,"il2cpp_field_get_type");
 if(!from_type||!field_type)return 0;
 ba.conveyors=dmethod("MoonCleaners","MoonWorld","get_BuiltConveyors","");
 ba.build_belt=dmethod("MoonCleaners","MoonWorld","TryBuildConveyor","MoonCleaners.MoonWalker;MoonCleaners.MoonWorld.ConveyorPlan;System.String&;System.Int32&");
 ba.free_pose=dmethod("MoonCleaners","MoonWorld","FreeGridPose","MoonCleaners.MoonFoundationModule;UnityEngine.Vector3;UnityEngine.Quaternion");
 ba.depot_pose=dmethod("MoonCleaners","MoonWorld","DepotPose","UnityEngine.Vector3;System.Single;UnityEngine.Vector3&;UnityEngine.Quaternion&;System.Int32");
 if(!ba.conveyors||!ba.build_belt||!ba.free_pose||!ba.depot_pose)return 0;
 ba.belt_class=from_type(da.param(ba.build_belt,1));ba.belt_ctor=ba.belt_class?get_method(ba.belt_class,".ctor",0):0;
 if(!ba.belt_ctor)return 0;
 const char *names[]={"start","startDirection","end","endDirection","startStructure","endStructure","startBelt","endBelt","replaces","startUp","endUp"};
 for(int i=0;i<11;i++){ba.belt_fields[i]=bp_field(ba.belt_class,names[i],i<4||i>8?"UnityEngine.Vector3":"System.Int32",field_type);if(!ba.belt_fields[i])return 0;}
 PTR snapshot=find_class("MoonCleaners","MoonConveyorSnapshot"),structure=from_type(da.param(bs.construction.begin_structure,0));
#define BF(F,K,N,T) ba.F=bp_field(K,N,T,field_type);if(!ba.F)return 0
 BF(start,snapshot,"start","UnityEngine.Vector3");BF(end,snapshot,"end","UnityEngine.Vector3");
 BF(start_direction,snapshot,"startDirection","UnityEngine.Vector3");BF(end_direction,snapshot,"endDirection","UnityEngine.Vector3");
 BF(plan_position,structure,"position","UnityEngine.Vector3");BF(plan_rotation,structure,"rotation","UnityEngine.Quaternion");
 BF(plan_free,structure,"free","System.Boolean");BF(plan_module,structure,"module","MoonCleaners.MoonFoundationModule");
 BF(support,find_class("MoonCleaners","MoonDepotSnapshot"),"support","System.Int32");
#undef BF
 return 1;
}
static int bp_grid_pose_matches(const Piece *piece,DVec point,Quat rotation){
 int ok=1;DVec planned={0};Quat expected={0};DRoot plan=0;
 if(piece->foundation){
  int support=-1,turn=0;unsigned char free_mode=0;float lift=0;DVec heading={0};
  void *args[]={(void *)&piece->kind,&point,(void *)&piece->data.pose.yaw,&support,&free_mode,&lift,&turn,&heading};
  plan=dhold(call(bs.construction.plan,dget(cm.world),args,&ok));if(!plan)return 0;
  field_value(dget(plan),ba.plan_position,&planned);field_value(dget(plan),ba.plan_rotation,&expected);
 }else{
  void *args[]={&point,(void *)&piece->data.pose.yaw,&planned,&expected,(void *)&piece->kind};call(ba.depot_pose,dget(cm.world),args,&ok);
 }
 float dot=rotation.x*expected.x+rotation.y*expected.y+rotation.z*expected.z+rotation.w*expected.w;
 float source_norm=rotation.x*rotation.x+rotation.y*rotation.y+rotation.z*rotation.z+rotation.w*rotation.w;
 float plan_norm=expected.x*expected.x+expected.y*expected.y+expected.z*expected.z+expected.w*expected.w;
 DVec error=ca(point,cx(planned,-1));ddrop(&plan);
 /* A yaw-only record must actually round-trip through the native grid/depot
  * planner at capture time. Unknown tilted/free subtypes cannot degrade to yaw. */
 return ok&&finite(dot)&&source_norm>.999f&&source_norm<1.001f&&plan_norm>.999f&&plan_norm<1.001f&&dot*dot>.99998f&&dot*dot<1.00002f&&cd(error,error)<.0004f;
}
static int bp_free_begin(MoonBuildPlacement *p,PTR world,const Piece *piece,MoonBuildPoint point){
 moon_build_placement_cancel(p);int ok=1,support=-1,turn=0;unsigned char free_mode=1;float lift=0;MoonBuildPoint heading={0};
 p->world=dhold(world);if(!p->world)return 0;p->kind=piece->kind;p->foundation=1;p->point=point;p->yaw=piece->data.pose.yaw;
 void *args[]={&p->kind,&point,&p->yaw,&support,&free_mode,&lift,&turn,&heading};
 p->plan=dhold(call(bs.construction.plan,world,args,&ok));PTR plan=dget(p->plan),module=0;unsigned char actual_free=0;
 if(plan&&ok){field_value(plan,ba.plan_free,&actual_free);field_value(plan,ba.plan_module,&module);}
 /* PlanStructure dispatches several free subtypes. Only a genuine free-grid
  * foundation is supported; never transplant an unknown subtype's plan. */
 void *pose[]={module,&point,(void *)&piece->data.pose.rotation};
 if(!actual_free||!module||!bp_bool(ba.free_pose,world,pose,&ok)||!ok){moon_build_placement_cancel(p);return 0;}
 cm.set_field(plan,ba.plan_position,&point);cm.set_field(plan,ba.plan_rotation,(void *)&piece->data.pose.rotation);
 void *check[]={plan};p->check=dhold(call(bs.construction.begin_structure,world,check,&ok));
 if(!ok||!p->check){moon_build_placement_cancel(p);return 0;}return 1;
}
static int bp_build_belt(const Piece *piece,PTR world,PTR walker){
 int ok=1;DRoot root=dhold(da.new_object(ba.belt_class));PTR plan=dget(root);if(!plan)return 0;
 call(ba.belt_ctor,plan,0,&ok);
 DVec start=ca(bs.destination,piece->offset),end=ca(bs.destination,piece->data.belt.end),zero={0};int none=0;
 void *values[]={&start,(void *)&piece->data.belt.start_direction,&end,(void *)&piece->data.belt.end_direction,&none,&none,&none,&none,&none,&zero,&zero};
 for(int i=0;i<11;i++)cm.set_field(plan,ba.belt_fields[i],values[i]);
 /* Snapshot path construction supplies zero up vectors. New identity fields
  * stay zero: no source IDs, replacement target or collision exemptions.
  * TryBuildConveyor itself runs CheckConveyor, then charges the native price. */
 PTR reason=0,result=0;int built_id=0;void *args[]={walker,plan,&reason,&built_id};
 if(ok)result=call(ba.build_belt,world,args,&ok);
 unsigned char *value=ok&&result?unbox(result):0;
 int built=ok&&value&&*value&&built_id>0;ddrop(&root);return built;
}

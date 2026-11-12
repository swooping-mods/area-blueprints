/* Whole-selection capture is staged until every borrowed record is retained. */
static int selected(DVec p){return valid(p)&&moon_build_in_box((MoonBuildPoint){p.x,p.y,p.z},(MoonBuildPoint){bs.a.x,bs.a.y,bs.a.z},(MoonBuildPoint){bs.b.x,bs.b.y,bs.b.z},2);}
static int capture_records(Blueprint *candidate,int belts,const char **failure){
 int ok=1;DRoot collection=dhold(call(belts?ba.conveyors:bs.depots,dget(cm.world),0,&ok));
 DRoot it=moon_build_enumerate(dget(collection),&ok);if(!collection||!it)ok=0;
 DVec origin={0};if(belts)origin=dvector(cm.position,ctr(dget(cm.world),&ok),&ok);
 for(int i=0;ok;i++){
  PTR borrowed=moon_build_next(it,cm.is_value,&ok);
  if(!ok||!borrowed)break;
  DRoot record=dhold(borrowed);PTR r=record?dget(record):0;
  /* A yielded record is part of this selection even when its retention fails.
   * Never publish a nonempty prefix as if the iterator had reached its end. */
  if(!r){ok=0;*failure="Selection record unavailable; previous slot retained";ddrop(&record);break;}
  if(i>=20000){ok=0;ddrop(&record);break;}
  Piece piece;dzero(&piece,sizeof(piece));DVec position={0};Quat rotation={0};
  if(belts){
   DVec end={0};field_value(r,ba.start,&position);field_value(r,ba.end,&end);position=ca(position,origin);end=ca(end,origin);
   if(!valid(position)||!valid(end)){*failure="Conveyor geometry unavailable; selection not saved";ok=0;ddrop(&record);break;}
   int first=selected(position),second=selected(end);if(first!=second){*failure="Belt crosses selection boundary; include both ends";ok=0;}
   if(!first||!second){ddrop(&record);if(!ok)break;continue;}
   piece.mode=BP_BELT;piece.offset=ca(position,cx(bs.a,-1));piece.data.belt.end=ca(end,cx(bs.a,-1));
   field_value(r,ba.start_direction,&piece.data.belt.start_direction);field_value(r,ba.end_direction,&piece.data.belt.end_direction);
  }else{
   void *pose[]={r,&position,&rotation};call(bs.pose,dget(cm.world),pose,&ok);
   if(!ok||!valid(position)){ok=0;ddrop(&record);break;}if(!selected(position)){ddrop(&record);continue;}
   int support=0;field_value(r,ba.support,&support);if(support>0||support< -1){*failure="Supported or stacked pieces need identity remapping";ok=0;ddrop(&record);break;}
   piece.kind=ci(r,"kind");void *kind[]={&piece.kind};piece.foundation=bp_bool(bs.is_foundation,dget(cm.world),kind,&ok);
   piece.offset=ca(position,cx(bs.a,-1));void *yaw[]={r};PTR y=ok?call(bs.yaw,dget(cm.world),yaw,&ok):0;float *value=ok&&y?unbox(y):0;if(!value)ok=0;else piece.data.pose.yaw=*value;
   void *free_args[]={r};int free_mode=bp_bool(bs.free_record,dget(cm.world),free_args,&ok);
   if(free_mode){if(!piece.foundation){*failure="Free-pose subtype unavailable; selection not saved";ok=0;}piece.mode=BP_FREE;piece.data.pose.rotation=rotation;}
   else if(ok&&!bp_grid_pose_matches(&piece,position,rotation)){*failure="Pose cannot round-trip through native grid/depot planning";ok=0;}
  }
  if(ok&&!bp_piece_valid(&piece)){*failure="Piece geometry invalid or outside the 200 m bound";ok=0;}
  if(ok&&candidate->count>=BP_MAX){*failure="Area too large: maximum 128 total pieces";ok=0;}
  if(ok)candidate->pieces[candidate->count++]=piece;
  ddrop(&record);
 }
 moon_build_end(&it,cm.is_value);ddrop(&collection);return ok;
}
static void capture(void){static Blueprint candidate;dzero(&candidate,sizeof(candidate));candidate.magic=BP_MAGIC;candidate.version=2;candidate.anchor=bs.a;
 const char *failure="Selection unavailable; previous slot retained";
 if(!capture_records(&candidate,0,&failure)||!capture_records(&candidate,1,&failure)){bs.status=failure;return;}
 if(!bp_valid(&candidate)){bs.status="Selection empty or invalid; previous slot retained";return;}
 /* Build foundations before depots, then belts. Support identity reconstruction
  * is deliberately rejected above rather than guessed from iteration order. */
 for(DWORD i=1;i<candidate.count;i++){Piece item=candidate.pieces[i];DWORD j=i;while(j&&item.foundation&&!candidate.pieces[j-1].foundation){candidate.pieces[j]=candidate.pieces[j-1];j--;}candidate.pieces[j]=item;}
 candidate.checksum=hash(&candidate,sizeof(candidate)-4);if(!iowrite(bs.path,&candidate,sizeof(candidate))){bs.status="Blueprint file could not be saved";return;}
 dcopy(&bp,&candidate,sizeof(bp));bs.status="Blueprint saved (buildings and belts)";sx_event("blueprint-saved",(int)bp.count);
}

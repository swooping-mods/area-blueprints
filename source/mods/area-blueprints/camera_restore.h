/* Restore the captured targets before releasing the shared camera lease. */
static int restore_survey(void *context){
 (void)context;int ok=1;PTR walker=dget(survey_walker);
 if(!survey_walker)return 1; /* A read-only capture may fail before retaining a target. */
 if(moon_camera_target_alive(sx.alive,walker,&ok)){unsigned char fly=(unsigned char)bs.was_flying,no=0;void *a[]={&bs.return_pos,&no};call(bs.teleport,walker,a,&ok);void *b[]={&fly,&no};call(bs.set_flying,walker,b,&ok);}return ok;
}
static void leave(void){bs.leaving=1;cancel();if(!moon_camera_finish(&survey_session,restore_survey,0))return;ddrop(&survey_walker);bs.editor=bs.leaving=bs.corner=0;hend();}

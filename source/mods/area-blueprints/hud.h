/* Persistent Canvas HUD avoids taking an exclusive OnGUI hook. */
typedef struct {float x,y;} HV2;
static struct {PTR ctor,add,go_tr,parent,canvas_mode,order,text,font,font_size,color,raycast,alignment,resources,anchor_min,anchor_max,pivot,anchored,size,destroy;DRoot root,label;} hu;
static inline void hs(PTR m,PTR o,void *v,int *ok){void *a[]={v};call(m,o,a,ok);}
static inline PTR hg(const char *name,PTR parent,int *ok){PTR o=da.new_object(find_class("UnityEngine","GameObject"));void *a[]={da.string(name)};call(hu.ctor,o,a,ok);if(parent){PTR tr=call(hu.go_tr,o,0,ok);unsigned char keep=0;void *b[]={parent,&keep};call(hu.parent,tr,b,ok);}return o;}
static inline PTR ha(PTR o,const char *ns,const char *name,int *ok){void *a[]={dtype(ns,name)};return call(hu.add,o,a,ok);}
static inline void hend(void){int ok=1;if(dget(hu.root)){void *a[]={dget(hu.root)};call(hu.destroy,0,a,&ok);}ddrop(&hu.root);ddrop(&hu.label);}
static inline void hshow(const char *s){int ok=1;if(!dget(hu.root)){
 PTR root=hg(MOD_NAME,0,&ok);hu.root=dhold(root);PTR canvas=ha(root,"UnityEngine","Canvas",&ok);int mode=0,order=290;hs(hu.canvas_mode,canvas,&mode,&ok);hs(hu.order,canvas,&order,&ok);
 PTR label=hg(MOD_NAME " status",call(hu.go_tr,root,0,&ok),&ok);PTR rect=ha(label,"UnityEngine","RectTransform",&ok),text=ha(label,"UnityEngine.UI","Text",&ok);hu.label=dhold(text);
 HV2 anchor={0,1},pos={18,-18},extent={630,150};hs(hu.anchor_min,rect,&anchor,&ok);hs(hu.anchor_max,rect,&anchor,&ok);hs(hu.pivot,rect,&anchor,&ok);hs(hu.anchored,rect,&pos,&ok);hs(hu.size,rect,&extent,&ok);
 void *a[]={dtype("UnityEngine","Font")};DRoot fonts=dhold(call(hu.resources,0,a,&ok));PTR font=fonts&&da.array_length(dget(fonts))?*(PTR*)darray(dget(fonts),0):0;if(font)hs(hu.font,text,font,&ok);else ok=0;ddrop(&fonts);int size=16,align=0;unsigned char no=0;DColor white={.95f,.96f,.97f,1};hs(hu.font_size,text,&size,&ok);hs(hu.alignment,text,&align,&ok);hs(hu.color,text,&white,&ok);hs(hu.raycast,text,&no,&ok);if(!ok){hend();return;}
 }void *a[]={da.string(s)};call(hu.text,dget(hu.label),a,&ok);}
static inline int hbind(void){
#define H(F,N,T,M,S) hu.F=dmethod(N,T,M,S);if(!hu.F)return 0
 H(ctor,"UnityEngine","GameObject",".ctor","System.String");H(add,"UnityEngine","GameObject","AddComponent","System.Type");H(go_tr,"UnityEngine","GameObject","get_transform","");H(parent,"UnityEngine","Transform","SetParent","UnityEngine.Transform;System.Boolean");H(canvas_mode,"UnityEngine","Canvas","set_renderMode","UnityEngine.RenderMode");H(order,"UnityEngine","Canvas","set_sortingOrder","System.Int32");H(text,"UnityEngine.UI","Text","set_text","System.String");H(font,"UnityEngine.UI","Text","set_font","UnityEngine.Font");H(font_size,"UnityEngine.UI","Text","set_fontSize","System.Int32");H(color,"UnityEngine.UI","Graphic","set_color","UnityEngine.Color");H(raycast,"UnityEngine.UI","Graphic","set_raycastTarget","System.Boolean");H(alignment,"UnityEngine.UI","Text","set_alignment","UnityEngine.TextAnchor");H(resources,"UnityEngine","Resources","FindObjectsOfTypeAll","System.Type");H(anchor_min,"UnityEngine","RectTransform","set_anchorMin","UnityEngine.Vector2");H(anchor_max,"UnityEngine","RectTransform","set_anchorMax","UnityEngine.Vector2");H(pivot,"UnityEngine","RectTransform","set_pivot","UnityEngine.Vector2");H(anchored,"UnityEngine","RectTransform","set_anchoredPosition","UnityEngine.Vector2");H(size,"UnityEngine","RectTransform","set_sizeDelta","UnityEngine.Vector2");H(destroy,"UnityEngine","Object","Destroy","UnityEngine.Object");
#undef H
 return 1;}

/***************************************************************************
 *   Copyright (C) 2008-2011 by Andreas Winter   *
 *   andreas.f.winter@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "undo_module.h"
#include "MainWindow.h"
#include "allWidgets.h"
#include "globals.h"
#include "xprotos.h"
#include "device.h"

using namespace std;

extern graph *g;

extern MainWindow * mainWin;
extern frmUndoList * FormUndoList;
extern bool undo_active;
extern int used_Nodes;
extern int akt_Node;
extern int NodeNr;
extern int max_node_nr;
extern class undo_node * Node;
extern DrawProps draw_props;
extern frmAxisProp * FormAxisProperties;
extern frmNonlinCurveFit * FormNonlinCurveFit;
extern Device_entry *device_table;
extern unsigned int ndevices;
extern frmPlotAppearance * FormPlotAppearance;
extern region undo_region;
extern int new_set_no;

char dummy[512];

//necessary for saving before and after state during modifications
int saved_prev_sets=0;
int saved_prev_graphs=0;
int saved_prev_tickmarks=0;
plotarr ** prev_sets=NULL;
graph ** prev_graphs=NULL;
tickmarks ** prev_tickmarks=NULL;
char ** old_strings=NULL;
int number_of_old_strings=0;
int old_data[16];//to store some data temporarely
double * old_ddata[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};//to store some data temporarely
int * old_idata[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};//to store some data temporarely
plotstr * old_tmstmp=NULL;
linetype * sav_lt=NULL;
boxtype * sav_bt=NULL;
ellipsetype * sav_et=NULL;
plotstr * sav_st=NULL;
Device_entry * saved_entry=NULL;
struct all_fit_settings old_fit_settings;
void * old_general_data=NULL;

bool wait_till_update=false;
bool dont_delete_saved_set_memory=false;

undo_node::undo_node(void)
{
data=NULL;
p=NULL;
type=UNDO_TYPE_INVALID;
    for (int i=0;i<MAX_DATA_IN_NODE;i++)
    {
    id[i]=NULL;
    ddata[i]=NULL;
    }
clearContents();//sets defaults
}

undo_node::~undo_node()
{
clearContents();
}

void undo_node::reStoreData(void)
{
int nid;
int offset;
linetype ** lt;
boxtype ** bt;
ellipsetype ** et;
plotstr ** st;
plotarr ** pa;
tickmarks ** tick;
graph ** gr;
char *ssav;
world w;
view v;
double * dd;
Datapoint * dp;
region * regio;
Device_entry ** dev_entr;
GLocator ** glt;
int findindex;
char ** old_str;
struct all_fit_settings * ps;
    switch (type)
    {
    case UNDO_TYPE_DATAPOINT_EDIT:
    dp=(Datapoint*)data;
    if (is_valid_setno(origin[0],origin[1]) && origin[3]>=0 && origin[3]<getsetlength(origin[0],origin[1]))
    {
        if (origin[3]==0)
        {
            if (this->active==true)//new datapoint --> delete datapoint
            {
                for (int i=origin[2]-1;i>=0;i--)
                del_point(origin[0],origin[1],id[0][i]);
            }
            else
            {
                for (int i=0;i<origin[2];i++)
                add_point_at(origin[0],origin[1],id[0][i],dp+i);
            }
        }
        else if (origin[3]==1)
        {
            if (this->active==true)//datapoint deleted --> insert datapoint
            {
                for (int i=0;i<origin[2];i++)
                add_point_at(origin[0],origin[1],id[0][i],dp+i);
            }
            else
            {
                for (int i=origin[2]-1;i>=0;i--)
                del_point(origin[0],origin[1],id[0][i]);
            }
        }
        else if (origin[3]==2)
        {
            if (this->active==true)//datapoint modified --> reinstall old point
            {//old=dp[0+j]
                for (int j=0;j<origin[2];j++)
                {
                for (int i=0;i<settype_cols(g[origin[0]].p[origin[1]].type);i++)
                g[origin[0]].p[origin[1]].data.ex[i][id[0][j]]=dp[j].ex[i];
                if (dp[j].s!=NULL)
                g[origin[0]].p[origin[1]].data.s[id[0][j]]=copy_string(g[origin[0]].p[origin[1]].data.s[id[0][j]],dp[j].s);
                }
            }
            else
            {//old=dp[len+j]
                for (int j=0;j<origin[2];j++)
                {
                for (int i=0;i<settype_cols(g[origin[0]].p[origin[1]].type);i++)
                g[origin[0]].p[origin[1]].data.ex[i][id[0][j]]=dp[j+origin[2]].ex[i];
                if (dp[j].s!=NULL)
                g[origin[0]].p[origin[1]].data.s[id[0][j]]=copy_string(g[origin[0]].p[origin[1]].data.s[id[0][j]],dp[j+origin[2]].s);
                }
            }
        }
    }
    break;
    case UNDO_TYPE_INVALID:
    break;
    case UNDO_TYPE_LOCATOR_FIXPOINT:
    glt=(GLocator **)data;
        if(this->active==true)//reinstall previous state
        offset=0;
        else
        offset=1;
            set_graph_locator(origin[0], glt[offset]);
    break;
    case UNDO_TYPE_HOTLINK:
    old_str=(char**)data;
        if(this->active==true)//reinstall previous state
        offset=0;
        else
        offset=1;
        for (int i=0;i<origin[2];i++)
        {
        if (!is_valid_setno(id[2][i],id[3][i])) continue;
        g[id[2][i]].p[id[3][i]].hotlink=id[0][i+offset*origin[2]];
            if (id[0][i+offset*origin[2]] && old_str[i+offset*origin[2]]!=NULL)
            {
            g[id[2][i]].p[id[3][i]].hotsrc=id[1][i+offset*origin[2]];
            strcpy(g[id[2][i]].p[id[3][i]].hotfile,old_str[i+offset*origin[2]]);
            }
            else
            {
            g[id[2][i]].p[id[3][i]].hotsrc=0;
            }
        }
    break;
    case UNDO_TYPE_SHOW_HIDE:
        if (origin[3]==UNDO_SET)
        {
        for (int i=0;i<origin[2];i++)
        set_set_hidden(id[0][i],id[1][i],!g[id[0][i]].p[id[1][i]].hidden);
        }
        else
        {
        for (int i=0;i<origin[2];i++)
        set_graph_hidden(id[0][i],!g[id[0][i]].hidden);
        }
    break;
    case UNDO_TYPE_SET_SWAP:
    swapset(origin[0],origin[1],origin[2],origin[3]);
    break;
    case UNDO_TYPE_GRAPH_SWAP:
    swap_graph(origin[0],origin[3]);
    break;
    case UNDO_TYPE_PACK_SETS:
    realloc_graph_plots(origin[0],origin[2]);
    pa=(plotarr **) data;
        for (int i=0;i<origin[2];i++)
        {
        reinstallSet(origin[0],i,pa[i],UNDO_COMPLETE);
        }
    if (this->active!=true)//make a redo
    packsets(origin[0]);
    break;
    case UNDO_TYPE_DEVICE_MODIFIED:
    dev_entr=(Device_entry**)data;
        if (this->active==true)//not undone yet --> reinstall previous stuff
        offset=0;
        else//reinstall state after change
        offset=1;
        set_device_props(origin[3],dev_entr[offset][0]);
        if (origin[0])//sync
        {
            for (int i=0;i<origin[1];i++)
            {
            device_table[i].pg.height=id[offset][i];
            device_table[i].pg.width=id[offset][origin[1]+i];
            }
        }
    break;
    case UNDO_TYPE_VIEWPORT_CHANGED:
        if (this->active==true)//not undone yet --> reinstall previous stuff
        offset=0;
        else//reinstall state after change
        offset=4;
        dd=(double*)data;
        v.xv1=dd[offset];
        v.xv2=dd[offset+1];
        v.yv1=dd[offset+2];
        v.yv2=dd[offset+3];
        set_graph_viewport(origin[0],v);
    break;
    case UNDO_TYPE_KILL_SET://reinstall set(s)
        pa=(plotarr **) data;
        for (int i=0;i<origin[2];i++)
        {
            if (is_set_active(id[0][i],id[1][i])==TRUE)//set exists -- this means, that the set-id hsa been reused
            {//get new set-id
            nid=nextset(id[0][i]);
            }
            else
            {//set-id may be used!
            nid=id[1][i];
            }
        id[1][i]=nid;
        reinstallSet(id[0][i],id[1][i],pa[i],origin[3]);
        }
    break;
    case UNDO_TYPE_NEW_SET://delete set(s)
        for (int i=0;i<origin[2];i++)
        {
        killset(id[0][i],id[1][i]);
        }
    break;
    case UNDO_TYPE_IMPORT_ASCII:
        if (this->active==true)//not undone yet --> delete set
        {
        killset(origin[0],origin[1]);
        }
        else
        {
            autoscale_onread=id[0][4];
            strcpy(dummy,savedFileName.toAscii());
        getdata(id[0][0], dummy, id[0][2], id[0][3]);
            id[0][1]=new_set_no;
            sprintf(dummy,"[G%d.S%d]",id[0][0],id[0][1]);
            this->Description=QObject::tr("Set data imported from ASCII ")+QString(dummy);
        }
    break;
    case UNDO_TYPE_IMPORT_BLOCK_DATA:
        if (this->active==true)//not undone yet --> delete set
        {
            if (this->p==NULL)
            killset(origin[0],origin[1]);
            else
            {//old_set_data present
            reinstallSet(origin[0],origin[1],p[0],UNDO_COMPLETE);
            }
        }
        else
        {
            autoscale_onread=id[0][4];
            strcpy(dummy,savedFileName.toAscii());
        getdata(id[0][0], dummy, id[0][2], id[0][3]);
        create_set_fromblock(id[0][0],id[0][1],id[0][3],id[1][0],id[2],id[1][1],id[0][4]);
            id[0][1]=new_set_no;
            sprintf(dummy,"[G%d.S%d]",id[0][0],id[0][1]);
            this->Description=QObject::tr("BlockData --> ")+QString(dummy);
        }
    break;
    case UNDO_TYPE_SET_MODIFIED:
    case UNDO_TYPE_SET_APPEARANCE:
        if (this->active==true)//not undone yet --> reinstall previous stuff
        {
        offset=0;
        nid=origin[1];
            //if (origin[1]<origin[2])
            //{
                for (int i=0;i<origin[2];i++)
                {
                findindex=ContainsGS(id[0],id[1],origin[1],id[0][origin[1]+i],id[1][origin[1]+i]);
                    if (findindex==-1)//new set not part of original sets
                    {
                    killsetdata(id[0][origin[1]+i],id[1][origin[1]+i]);
                    }
                }
            //}
        }
        else//reinstall state after change
        {
        offset=origin[1];
        nid=origin[2];
            //if (origin[2]<origin[1])
            //{
            ///cout << "old sets have been more-->delete some! (we want the new states)" << endl;
                for (int i=0;i<origin[1];i++)//go through all old sets
                {
                    if (ContainsGS(id[0]+origin[1],id[1]+origin[1],origin[2],id[0][i],id[1][i])==-1)
                    {//an old set is not part of the new sets list --> delete it
                    killsetdata(id[0][i],id[1][i]);
                    }
                }
            //}
        }
        pa=(plotarr **) data;
            if (nid==0)//no more sets afterwards
            {
                for (int i=0;i<origin[2];i++)
                {
                killsetdata(id[0][i],id[1][i]);
                }
            }
            else
            {
                for (int i=0;i<nid;i++)//in this case: changing the set-ids is not apropriate
                {
                reinstallSet(id[0][i+offset],id[1][i+offset],pa[offset+i],origin[3]);
                    if (is_set_active(id[0][i+offset],id[1][i+offset])==FALSE)
                    {
                    set_set_hidden(id[0][i+offset],id[1][i+offset], FALSE);
                    }
                }
            }
    break;
    case UNDO_TYPE_KILL_GRAPH:
        gr=(graph**)data;
        for (int i=0;i<origin[2];i++)
        {
        if (graph_allocate(this->id[0][i])==RETURN_FAILURE)
        {
        cout << "Could not allocate new graph! " << this->id[0][i] << endl;
        }
        else
        {
        reinstallGraph(this->id[0][i],gr[i],origin[3]);
        }
        }
    break;
    case UNDO_TYPE_NEW_GRAPH:
        for (int i=0;i<origin[2];i++)
        {
        kill_graph(this->id[0][i]);
        }
    break;
    case UNDO_TYPE_GRAPH_APPEARANCE:///Graph Sachen fehlen!
        gr=(graph**)data;
        if (this->active==true)//not undone yet --> reinstall previous stuff
        {
        offset=0;
        nid=origin[1];
        }
        else//reinstall state after change
        {
        offset=origin[1];
        nid=origin[2];
        }
        //cout << "number_of_graphs=" << number_of_graphs() << " nid=" << nid << endl;
            /*while (number_of_graphs()>nid)
            kill_graph(number_of_graphs()-1);*/
            while (number_of_graphs()<nid)
            graph_allocate(number_of_graphs());
        for (int i=0;i<nid;i++)//in this case: changing the set-ids is not apropriate
        {
        reinstallGraph(id[0][i+offset],gr[offset+i],origin[3]);
        }
    break;
    case UNDO_TYPE_PLOT_APPEARANCE:
        st=(plotstr**)data;
        if (this->active==true)//not undone yet --> reinstall previous stuff
        offset=0;
        else//reinstall state after change
        offset=2;
        draw_props.bgcolor=origin[offset];
        draw_props.bgfilled=origin[offset+1];
        ssav=timestamp.s;
        memcpy(&timestamp,st[offset/2],sizeof(plotstr));
        timestamp.s=ssav;
        if (FormPlotAppearance!=NULL) FormPlotAppearance->init();
    break;
    case UNDO_TYPE_OBJECT_MODIFIED:
    if (this->active==true)//not undone yet --> reinstall previous stuff
    offset=0;
    else//reinstall state after change
    offset=1;
    nid=origin[0];
        switch (origin[1])//type
        {
        case OBJECT_BOX:
        bt=(boxtype**)data;
            if (isactive_box(nid))
            memcpy(boxes+nid,bt[offset],sizeof(boxtype));
        break;
        case OBJECT_LINE:
        lt=(linetype**)data;
            if (isactive_line(nid))
            memcpy(lines+nid,lt[offset],sizeof(linetype));
        break;
        case OBJECT_ELLIPSE:
        et=(ellipsetype**)data;
            if (isactive_ellipse(nid))
            memcpy(ellip+nid,et[offset],sizeof(ellipsetype));
        break;
        case OBJECT_STRING:
        st=(plotstr**)data;
            if (isactive_string(nid))
            {
            memcpy(pstr+nid,st[offset],sizeof(plotstr));
                if (pstr[nid].s!=NULL)
                {
                pstr[nid].s=copy_string(NULL,(st[offset])->s);
                }
            }
        break;
        }
    break;
    case UNDO_TYPE_PAGE_SETUP:
    break;
    case UNDO_TYPE_AXIS_PROPERTIES:
        tick=(tickmarks **)data;
        if (this->active==true)//not undone yet --> reinstall previous stuff
        offset=0;
        else//reinstall state after change
        offset=origin[2];

        //cout << "offset=" << offset << endl;

        //id[0][0]=axis_start;
        //id[0][1]=axis_stop;
        //id[0][2]=graph_start;
        //id[0][3]=graph_stop;

        for (int i=id[0][2];i<=id[0][3];i++)//graphs
        {
            for (int j=id[0][0];j<=id[0][1];j++)//axes
            {
            set_graph_tickmarks(i,j,tick[offset]);
            w.xg1=ddata[0][offset];
            w.xg2=ddata[1][offset];
            w.yg1=ddata[2][offset];
            w.yg2=ddata[3][offset];
            g[i].xscale = id[1][4*offset+0];
            g[i].yscale = id[1][4*offset+1];
            g[i].xinvert = id[1][4*offset+2];
            g[i].yinvert = id[1][4*offset+3];
            offset++;
            }
            //set_graph_xscale(i,id[1][4*offset+0]);
            //set_graph_yscale(i,id[1][4*offset+1]);
            set_graph_world(i, w);
        }
        if (FormAxisProperties!=NULL)
        FormAxisProperties->update_ticks(get_cg());

        //cout << "offset=" << offset << endl;
    break;
    case UNDO_TYPE_NEW_ORDERING:
        if (origin[3]==PUSH_SET_TOFRONT)
        {
            if (this->active==false)//already undone
            {
            pushset(origin[0],origin[1],PUSH_SET_TOFRONT);
            }
            else
            {//set is at the end of the list
                for (int i=number_of_sets(origin[0]) - 1;i>origin[1];i--)
                {
                swapset(origin[0], i, origin[0], i - 1);
                }
            }
        }
        else
        {
            if (this->active==false)//already undone
            {
            pushset(origin[0],origin[1],PUSH_SET_TOBACK);
            }
            else
            {//set is at the beginning of the list
                for (int i=0;i<origin[1];i++)
                {
                swapset(origin[0], i, origin[0], i + 1);
                }
            }
        }
    break;
    case UNDO_TYPE_FIT:
    ps=(struct all_fit_settings *)data;
        if (active==true)
        {
        offset=0;
        }
        else
        {
        offset=1;
        }
            if (this->p[offset]!=NULL)
            {
            reinstallSet(ps[offset].dest_gno,ps[offset].dest_sets,this->p[offset],UNDO_COMPLETE);
            }
            else
            {
                if (!is_valid_setno(ps[offset].dest_gno,ps[offset].dest_sets))
                {
                killset(ps[!offset].dest_gno,ps[!offset].dest_sets);
                }
                else
                {
                killset(ps[offset].dest_gno,ps[offset].dest_sets);
                }
            }
        set_fit_settings(ps+offset);
    break;
    case UNDO_TYPE_REGION:
        regio=(region*)data;
        if (origin[3]==0)//modified
        {
            if (active==true)//reinstall old region
            CopyRegion(regio,rg+origin[0]);
            else//reinstall new region
            CopyRegion(regio+1,rg+origin[0]);
        }
        else//killed
        {
            if (origin[2]==MAXREGION)
            {
                if (active==true)//reinstall old regions
                {
                    for (int i=0;i<MAXREGION;i++)
                    CopyRegion(regio+i,rg+i);
                }
                else//kill all regions
                {
                    for (int i=0;i<MAXREGION;i++)
                    kill_region(i);
                }
            }
            else
            {
                if (active==true)//reinstall old region
                CopyRegion(regio,rg+origin[0]);
                else
                kill_region(origin[0]);
            }
        }
    break;
    case UNDO_TYPE_OBJECT_INSERTED:
        for (int i=0;i<origin[2];i++)
        {
            switch (origin[1])//type
            {
            case OBJECT_BOX:
            kill_box(id[0][i]);
            break;
            case OBJECT_LINE:
            kill_line(id[0][i]);
            break;
            case OBJECT_ELLIPSE:
            kill_ellipse(id[0][i]);
            break;
            case OBJECT_STRING:
            kill_string(id[0][i]);
            break;
            }
        }
    break;
    case UNDO_TYPE_OBJECT_DELETED:
        switch (origin[1])//type
        {
        case OBJECT_BOX:
        bt=(boxtype**)data;
            for (int i=0;i<origin[2];i++)
            {
            nid=next_box();
            id[0][i]=nid;
            memcpy(boxes+nid,bt[i],sizeof(boxtype));
            }
        break;
        case OBJECT_LINE:
        lt=(linetype**)data;
            for (int i=0;i<origin[2];i++)
            {
            nid=next_line();
            id[0][i]=nid;
            memcpy(lines+nid,lt[i],sizeof(linetype));
            }
        break;
        case OBJECT_ELLIPSE:
        et=(ellipsetype**)data;
            for (int i=0;i<origin[2];i++)
            {
            nid=next_ellipse();
            id[0][i]=nid;
            memcpy(ellip+nid,et[i],sizeof(ellipsetype));
            }
        break;
        case OBJECT_STRING:
        st=(plotstr**)data;
            for (int i=0;i<origin[2];i++)
            {
            nid=next_string();
            id[0][i]=nid;
            memcpy(pstr+nid,st[i],sizeof(plotstr));
                if (pstr[nid].s!=NULL)
                {
                pstr[nid].s=copy_string(NULL,(st[i])->s);
                }
            }
        break;
        }
    break;
    }
this->active=!(this->active);
this->type=get_inverse_action(this->type);
    if (wait_till_update==false)
    {
    mainWin->mainArea->completeRedraw();
    }
}

void undo_node::clearContents(void)
{
    int nr;
    linetype ** lt;
    boxtype ** bt;
    ellipsetype ** et;
    plotstr ** st;
    plotarr ** pa;
    graph ** gr;
    tickmarks ** tick;
    double * dd;
    Datapoint * dp;
    region * regio;
    Device_entry ** dev_entr;
    GLocator ** glt;
    char ** old_str;
if (data!=NULL)
switch (type)
{
case UNDO_TYPE_DATAPOINT_EDIT:
    dp=(Datapoint*)data;
    for (int i=0;i<2*origin[2];i++)
    xfree(dp[i].s);
    delete[] dp;
break;
case UNDO_TYPE_PACK_SETS:
    pa=(plotarr**)data;
    for (int i=0;i<origin[2];i++)
    deleteSavedSet(pa[i],UNDO_COMPLETE);
    delete[] pa;
break;
case UNDO_TYPE_VIEWPORT_CHANGED:
    dd=(double*)data;
    delete[] dd;
break;
case UNDO_TYPE_PLOT_APPEARANCE:
    st=(plotstr**)data;
    delete st[0];
    delete st[1];
    delete[] st;
break;
case UNDO_TYPE_DEVICE_MODIFIED:
    dev_entr=(Device_entry**)data;
    delete[] dev_entr[0];
    delete[] dev_entr[1];
    delete[] dev_entr;
break;
case UNDO_TYPE_OBJECT_INSERTED:
case UNDO_TYPE_OBJECT_DELETED:
case UNDO_TYPE_OBJECT_MODIFIED:
if (type==UNDO_TYPE_OBJECT_MODIFIED)
nr=2*origin[2];
else
nr=origin[2];
    switch  (origin[1])//type
    {
    case OBJECT_BOX:
    bt=(boxtype**)data;
        for (int i=0;i<nr;i++)
        delete[] bt[i];
    break;
    case OBJECT_LINE:
    lt=(linetype**)data;
        for (int i=0;i<nr;i++)
        delete[] lt[i];
    break;
    case OBJECT_ELLIPSE:
    et=(ellipsetype**)data;
        for (int i=0;i<nr;i++)
        delete[] et[i];
    break;
    case OBJECT_STRING:
    st=(plotstr**)data;
        for (int i=0;i<nr;i++)
        {
        XCFREE(st[i]->s);
        delete[] st[i];
        }
    break;
    }
break;
case UNDO_TYPE_LOCATOR_FIXPOINT:
    glt=(GLocator **)data;
    delete glt[0];
    delete glt[1];
    delete[] glt;
break;
case UNDO_TYPE_HOTLINK:
    old_str=(char**)data;
    for (int i=0;i<origin[2];i++)
    {
    if (old_str[i]!=NULL) delete[] old_str;
    }
    delete[] old_str;
break;
case UNDO_TYPE_IMPORT_BLOCK_DATA:
    if (p!=NULL)
    {
    deleteSavedSet(p[0],UNDO_COMPLETE);
    delete[] p;
    }
break;
case UNDO_TYPE_KILL_SET:
case UNDO_TYPE_NEW_SET:
case UNDO_TYPE_SET_APPEARANCE:
if (type==UNDO_TYPE_KILL_SET || type==UNDO_TYPE_NEW_SET)
nr=origin[2];
else
nr=origin[1]+origin[2];
    pa=(plotarr**)data;
    for (int i=0;i<nr;i++)
    deleteSavedSet(pa[i],origin[3]);
    delete[] pa;
break;
case UNDO_TYPE_KILL_GRAPH:
case UNDO_TYPE_NEW_GRAPH:
case UNDO_TYPE_GRAPH_APPEARANCE:
if (type==UNDO_TYPE_KILL_GRAPH || type==UNDO_TYPE_NEW_GRAPH)
nr=origin[2];
else
nr=origin[1]+origin[2];
    gr=(graph**)data;
    for (int i=0;i<nr;i++)
    deleteSavedGraph(gr[i],origin[3]);
    delete[] gr;
break;
case UNDO_TYPE_AXIS_PROPERTIES:
    tick=(tickmarks**)data;
nr=2*origin[2];
    for (int i=0;i<nr;i++)
    free_graph_tickmarks(tick[i]);
    delete[] tick;
break;
case UNDO_TYPE_REGION:
regio=(region*)data;
    if (origin[3]==0)//modified
    {
    DeleteRegion(regio);
    DeleteRegion(regio+1);
    }
    else if (origin[3]==1)//killed
    {
        if (origin[0]==MAXREGION)
        {
        for (int i=0;i<MAXREGION;i++)
        DeleteRegion(regio+i);
        }
        else
        {
        DeleteRegion(regio);
        }
    }
    delete[] regio;
break;
//OTHER CASES
}
    type=UNDO_TYPE_INVALID;
    saved=active=multiple=false;
    savedFileName=AdditionalDescription=Description=QString();
    data=NULL;
        if (p!=NULL)
        {
            for (int i=0;i<origin[2];i++)
            {
            deleteSavedSet(p[i],UNDO_COMPLETE);
            //delete p[i];
            }
        delete[] p;
        }
    p=NULL;
    for (int i=0;i<MAX_DATA_IN_NODE;i++)
    {
    origin[i]=0;
    if (id[i]!=NULL) delete[] id[i];
    id[i]=NULL;
    if (ddata[i]!=NULL) delete[] ddata[i];
    ddata[i]=NULL;
    }
}

QString undo_node::getDescription(void)
{
QString st("");

if (this_node_nr==nextUndoNode())
st+="U | ";
else if (this_node_nr==nextRedoNode())
st+="R | ";
else
st+="  | ";

if (active==false)
{
st+="(*) ";
}
st+=Description;
if (multiple==true)
{
st+=" (multiple)";
}
if (!AdditionalDescription.isEmpty())
{
st+=" ("+AdditionalDescription+")";
}
return st;
}

int undo_node::containsSet(int gno,int setno)
{
int ret=-1;
graph ** gr;
switch (this->type)
{
case UNDO_TYPE_KILL_SET:
case UNDO_TYPE_NEW_SET:
    for (int i=0;i<origin[2];i++)
    {
        if (id[0][i]==gno && id[1][i]==setno)
        {
        ret=i;
        break;
        }
    }
break;
case UNDO_TYPE_SET_MODIFIED:
    for (int i=0;i<origin[2];i++)//koennte auch origin[1] sein
    {
        if (id[0][i]==gno && id[1][i]==setno)
        {
        ret=i;
        break;
        }
    }
break;
case UNDO_TYPE_KILL_GRAPH:
case UNDO_TYPE_NEW_GRAPH:
gr=(graph**)data;
    for (int i=0;i<origin[2];i++)
    {
        if (id[0][i]==gno && gr[i]->maxplot>setno)
        {
            ret=i;
            break;
        }
    }
break;
case UNDO_TYPE_GRAPH_APPEARANCE:
if (origin[3]==UNDO_COMPLETE && origin[3]==UNDO_DATA)
{
    gr=(graph**)data;
        for (int i=0;i<origin[2];i++)//koennte auch origin[1] sein
        {
            if (id[0][i]==gno && gr[i]->maxplot>setno)
            {
                ret=i;
                break;
            }
        }
}
break;
default:
break;
}
return ret;
}

void ObjectsDeleted(int len,int * ids,int type)
{
undo_node * nn;
if (undo_active==false) return;

wait_till_update=true;
ObjectsCreated(len,ids,type);
wait_till_update=false;

nn=getPreviousNode();//go  back to set some stuff

nn->type=UNDO_TYPE_OBJECT_DELETED;//inverse action
nn->Description=nn->Description.replace(0,QObject::tr("Insert").size(),QObject::tr("Delete"));//inverse description

updateUndoList();
}

void ObjectsCreated(int len,int * ids,int type)
{
linetype ** lt;
boxtype ** bt;
ellipsetype ** et;
plotstr ** st;
undo_node * nn;
if (undo_active==false) return;
nn=getNextNode();
nn->clearContents();//to be on the save side
    if (len>1)
    {
    nn->multiple=true;
    }
    else
    {
    nn->multiple=false;
    }
nn->type=UNDO_TYPE_OBJECT_INSERTED;
nn->saved=false;
nn->origin[0]=ids[0];//which object was it?
nn->origin[1]=type;//what type of object was it?
nn->origin[2]=len;
nn->active=true;//active=false means: "already undone", active=true means: "ready to be undone"
nn->id[0]=new int[len];
memcpy(nn->id[0],ids,sizeof(int)*len);
switch (type)
{
case OBJECT_BOX:
bt=new boxtype*[len];
nn->Description=QObject::tr("Insert Box");
    for (int i=0;i<len;i++)
    {
    bt[i]=new boxtype;
    memcpy(bt[i],boxes+ids[i],sizeof(boxtype));
    }
nn->data=(void*)bt;
break;
case OBJECT_LINE:
nn->Description=QObject::tr("Insert Line");
lt=new linetype*[len];
    for (int i=0;i<len;i++)
    {
    lt[i]=new linetype;
    memcpy(lt[i],lines+ids[i],sizeof(linetype));
    }
nn->data=(void*)lt;
    break;
case OBJECT_ELLIPSE:
nn->Description=QObject::tr("Insert Ellipse");
et=new ellipsetype*[len];
    for (int i=0;i<len;i++)
    {
    et[i]=new ellipsetype;
    memcpy(et[i],ellip+ids[i],sizeof(ellipsetype));
    }
nn->data=(void*)et;
break;
case OBJECT_STRING:
nn->Description=QObject::tr("Insert String");
st=new plotstr*[len];
    for (int i=0;i<len;i++)
    {
    st[i]=new plotstr;
    memcpy(st[i],pstr+ids[i],sizeof(plotstr));
        if (st[i]->s!=NULL)
        {
        st[i]->s=new char[strlen(pstr[ids[i]].s)+1];
        strcpy(st[i]->s,pstr[ids[i]].s);
        }
    }
nn->data=(void*)st;
break;
}
NextNode();//finish writing and prepare the next node
if (wait_till_update==false) updateUndoList();
}

void Undo(void)
{
if (UndoPossible()==false) return;
wait_till_update=true;
Node[akt_Node].reStoreData();//make the undo-command
if (akt_Node==0 && used_Nodes==max_node_nr)//wrap around has occured
{
    if (NodeNr==0)
    akt_Node=-1;//end reached
    else
    akt_Node=max_node_nr-1;
}
else if (akt_Node==NodeNr) akt_Node=-1;//end reached with wrap around
else akt_Node--;
CheckActive();
wait_till_update=false;
mainWin->mainArea->completeRedraw();
}

void Redo(void)
{
if (RedoPossible()==false) return;
wait_till_update=true;
if (akt_Node==-1 && used_Nodes>0)//Nodes present and Undo done a lot of times --> ReDo the first Node saved
{
    if (used_Nodes==max_node_nr)//wrap around has occured
    {
    akt_Node=NodeNr-1;
    Node[NodeNr].reStoreData();
    }
    else
    Node[0].reStoreData();
}
else
{
    if (akt_Node==max_node_nr-1)//next undo would be last in memory --> Redo would be first in memory (wrap around)
    Node[0].reStoreData();
    else
    Node[akt_Node+1].reStoreData();
}
akt_Node++;
if (akt_Node==max_node_nr) akt_Node=0;
CheckActive();
wait_till_update=false;
mainWin->mainArea->completeRedraw();
}

bool UndoPossible(void)
{
if (used_Nodes==0) return false;//no Nodes to Undo
    if (used_Nodes>0)
    {
        if (akt_Node==-1)
        return false;//everything has been made undone --> nothing left
        else
        return true;//akt_node>=0 --> there is a node that can be made undone
    }
    else
    {
    cout << "used_Nodes<0 !?" << endl;
    }
return false;
}

bool RedoPossible(void)
{
if (used_Nodes==0) return false;//no Nodes to Undo or Redo
    if (used_Nodes>0)
    {
        if ((akt_Node==NodeNr-1 && NodeNr>0) || (NodeNr==0 && akt_Node==max_node_nr-1))//nothing has been made undone yet --> nothing to redo
        return false;
        else
        return true;//akt_node>=0 --> there is a node that can be made undone
    }
    else
    {
    cout << "used_Nodes<0 !?" << endl;
    }
return false;
}

void CheckActive(void)
{
    /*cout << ">> NodeNr=" << NodeNr << endl;
    cout << "akt_Node=" << akt_Node << endl;
    cout << "used_Nodes=" << used_Nodes << endl;
    cout << "undo=" << UndoPossible() << "(" << nextUndoNode() << ")" << " redo=" << RedoPossible() << "(" << nextRedoNode() << ")" << endl;*/
    if (mainWin!=NULL)
    {
    mainWin->actUndo->setEnabled(UndoPossible());
    mainWin->actRedo->setEnabled(RedoPossible());
    }
    if (FormUndoList!=NULL)
    {
    FormUndoList->init();
    //if (FormUndoList->isVisible())
    FormUndoList->repaint();
    }
}

void PreviousNode(void)
{
    NodeNr--;//this will not change anything except the NodeNr
    if (NodeNr<0)
    {
    NodeNr=max_node_nr-1;
    }
}

void NextNode(void)//increase Nodes
{
    akt_Node=NodeNr;//the node that is to be undone next time is the one we wrote to this time!
    NodeNr++;
    used_Nodes++;
    if (used_Nodes>=max_node_nr)
    used_Nodes=max_node_nr;//used_nodes never greater then max_node_nr

    if (NodeNr>=max_node_nr) //wrap around
    {
    NodeNr=0;//next Node to write to is first Node in memory
    }
    CheckActive();
}

void initNodes(void)
{
if (Node==NULL) Node=new undo_node[MAX_UNDO_NODES];
    for (int i=0;i<MAX_UNDO_NODES;i++)
    {
    Node[i].clearContents();
    Node[i].this_node_nr=i;//necessary to know!
    }
NodeNr=0;
akt_Node=-1;//no active nodes, no undo, mo redo
used_Nodes=0;
max_node_nr=MAX_UNDO_NODES;
CheckActive();
}

undo_node * getPreviousNode(void)//get the node it has been last written to
{
undo_node * nn=NULL;
    if (used_Nodes==0)//this should never happen here!
    {
    nn=Node;//makes no sense!
    }
    else if (used_Nodes==max_node_nr)//all available space used
    {
        if (NodeNr==0)
        nn=Node+max_node_nr-1;
        else
        nn=Node+NodeNr-1;
    }
    else if (NodeNr>0 && NodeNr<max_node_nr)
    {
    nn=Node+NodeNr-1;
    }
    else
    {
    cout << "Error in getPreviousNode" << endl;
    cout << "NodeNr=" << NodeNr << endl;
    cout << "max_node_nr=" << max_node_nr << endl;
    cout << "used_Nodes=" << used_Nodes << endl;
    cout << "akt_Node=" << akt_Node << endl;
    }
return nn;
}

undo_node * getNextNode(void)//get Node to write into next
{
undo_node * nn=NULL;
    if (used_Nodes==0)
    nn=Node;//we start at the beginning
    else if (NodeNr>=0 && NodeNr<max_node_nr)
    nn=Node+NodeNr;
    else
    {
    cout << "Error in getNextNode" << endl;
    cout << "NodeNr=" << NodeNr << endl;
    cout << "max_node_nr=" << max_node_nr << endl;
    cout << "used_Nodes=" << used_Nodes << endl;
    cout << "akt_Node=" << akt_Node << endl;
    }
return nn;
}

int getNrOfNodes(void)
{
return used_Nodes;
}

QString getNodeDescription(int i)
{
return Node[index_translator(i)].getDescription();
}

int index_translator(int i)
{
static int j;
j=i;
if (used_Nodes<max_node_nr)//normal order
{
j=i;
}
else//wrap around occured
{
    if (NodeNr+i<max_node_nr)
    j=NodeNr+i;
    else
    j=NodeNr+i-max_node_nr;
}
return j;
}

int nextUndoNode(void)
{
static int ret;
ret=-1;
if (used_Nodes==0 || UndoPossible()==false)
ret=-1;
else
ret=akt_Node;
return ret;
}

int nextRedoNode(void)
{
static int ret;
ret=-1;
if (used_Nodes==0 || RedoPossible()==false) ret=-1;//nothing
else
{
ret=akt_Node+1;
if (akt_Node==-1 && used_Nodes==max_node_nr)
{
ret=NodeNr;
}
else if (akt_Node==-1)
{
ret=0;
}
if (ret==max_node_nr) ret=0;
if (akt_Node==NodeNr-1 && used_Nodes==max_node_nr && NodeNr>0) ret=-1;
}
return ret;
}

void SetsDeleted(int len,int * gnos,int * snos,int what)
{
if (undo_active==false) return;
undo_node * nn=getNextNode();
nn->clearContents();

nn->type=UNDO_TYPE_KILL_SET;
nn->active=true;
if (what==UNDO_DATA)
{
nn->Description=QObject::tr("Kill set data");
}
else
{
nn->Description=QObject::tr("Kill set");
}
    if (len>1)
    {
    nn->multiple=true;
    }
    else
    {
    nn->multiple=false;
    sprintf(dummy,"[G%d.S%d]",gnos[0],snos[0]);
    nn->Description+=QString(dummy);
    }
nn->origin[0]=gnos[0];
nn->origin[1]=snos[0];
nn->origin[2]=len;
nn->origin[3]=what;
nn->id[0]=new int[len];
nn->id[1]=new int[len];
memcpy(nn->id[0],gnos,len*sizeof(int));
memcpy(nn->id[1],snos,len*sizeof(int));
if (what==UNDO_DATA || what==UNDO_COMPLETE)
{
    plotarr ** pa=new plotarr*[len];
    for (int i=0;i<len;i++)
    {
    pa[i]=new plotarr;
    copySet(gnos[i],snos[i],pa[i],what);
    }
    nn->data=(void*) pa;
}
else
nn->data=NULL;

NextNode();//finish writing and prepare the next node
if (wait_till_update==false) updateUndoList();
}

void SetsCreated(int len,int * gnos,int * snos,int what)
{
if (undo_active==false) return;
wait_till_update=true;
SetsDeleted(len,gnos,snos,what);
wait_till_update=false;
undo_node * nn=getPreviousNode();//go  back to set some stuff

nn->type=UNDO_TYPE_NEW_SET;//inverse action
nn->Description=nn->Description.replace(0,QObject::tr("Kill").size(),QObject::tr("New"));//inverse description

updateUndoList();
}

void GraphsDeleted(int len,int * gnos,int what)
{
if (undo_active==false) return;
//int col;
undo_node * nn=getNextNode();
nn->clearContents();
graph ** gr=new graph*[len];
nn->type=UNDO_TYPE_KILL_GRAPH;
nn->active=true;
nn->Description=QObject::tr("Kill graph");
    if (len>1)
    {
    nn->multiple=true;
    }
    else
    {
    nn->multiple=false;
    sprintf(dummy,"[G%d]",gnos[0]);
    nn->Description+=QString(dummy);
    }
nn->origin[2]=len;
nn->origin[0]=gnos[0];
nn->origin[3]=what;
nn->id[0]=new int[len];//the graph ids
    for (int i=0;i<len;i++)
    {
    nn->id[0][i]=gnos[i];
    gr[i]=new graph;
    copyGraph(gnos[i],gr[i],what);
    }
nn->data=(void*)gr;
NextNode();//finish writing and prepare the next node
if (wait_till_update==false) updateUndoList();
}

void GraphsCreated(int len,int * gnos,int what)
{
if (undo_active==false) return;
wait_till_update=true;
GraphsDeleted(len,gnos,what);
wait_till_update=false;
undo_node * nn=getPreviousNode();//go  back to set some stuff

nn->type=UNDO_TYPE_NEW_GRAPH;//inverse action
nn->Description=nn->Description.replace(0,QObject::tr("Kill").size(),QObject::tr("New"));//inverse description

updateUndoList();
}

void copySet(int gno,int setno,plotarr * pa,int what)
{
int i;
int cols;
if (!is_valid_setno(gno, setno)) return;
memcpy(pa,g[gno].p+setno,sizeof(plotarr));//copy the whole plotarr! --> always copy this!
//only the actual address of the set-data is coppied --> copy contents as well
cols=settype_cols(pa->type);
    for (i=0;i<MAX_SET_COLS;i++)
    pa->data.ex[i]=NULL;//set to invalid values!
    pa->data.s=NULL;//set to invalid values!
if (what==UNDO_COMPLETE || what&UNDO_DATA)//do not copy data if only appearance is asked
{
for (i=0;i<cols;i++)//copy coulmns
{
pa->data.ex[i]=new double[pa->data.len];
///g[gno].p[setno].data.ex[i] = (double *)xrealloc(g[gno].p[setno].data.ex[i],pa->data.len*sizeof(double));
memcpy(pa->data.ex[i],g[gno].p[setno].data.ex[i],(pa->data.len)*sizeof(double));
}
    if (pa->data.s!=NULL)//copy strings
    {
    pa->data.s = new char*[pa->data.len];
        for (i=0;i<pa->data.len;i++)
        {
        pa->data.s[i]=new char[strlen(g[gno].p[setno].data.s[i]) + 1];
        strcpy(pa->data.s[i], g[gno].p[setno].data.s[i]);
        }
    }
        for (i=cols;i<MAX_SET_COLS;i++)
        {
        pa->data.ex[i]=NULL;
        }
}//End UNDO_COMPLETE / UNDO_DATA
}

void deleteSavedSet(plotarr * pa,int what)//clear all entries including the deep copy of the data
{
int i;
int cols;
if (pa==NULL) return;
cols=settype_cols(pa->type);
if (what==UNDO_COMPLETE || what&UNDO_DATA)//do not delete data if only appearance is asked (because not data has been allocated)
{
        for (i=0;i<cols;i++)//copy coulmns
        delete[] pa->data.ex[i];
    if (pa->data.s!=NULL)
    {
        for (i=0;i<pa->data.len;i++)
        delete[] pa->data.s[i];
    delete[] pa->data.s;
    }
}
    if (dont_delete_saved_set_memory==false)
    {
    delete pa;
    pa=NULL;
    }
}

void reinstallSet(int gno,int setno,plotarr * pa,int what)
{
int i;
int cols,old_cols;
bool activ;
bool valid=is_valid_setno(gno,setno);
if (pa==NULL) return;//there is nothing to reinstall
    if (valid==FALSE)
    {
    int alloc=allocate_set(gno,setno);
        if (alloc!=RETURN_SUCCESS)
        {
        cout << "invalid set: G" << gno << ".S" << setno << ", unable to restore yet!" << endl;
        return;
        }
    }
old_cols=dataset_cols(gno, setno);
cols=settype_cols(pa->type);
activ=is_set_active(gno,setno);

char ** sav_s=g[gno].p[setno].data.s;
double * sav_ex[MAX_SET_COLS];
int sav_type=g[gno].p[setno].type;
int sav_length=g[gno].p[setno].data.len;
memcpy(sav_ex,g[gno].p[setno].data.ex,sizeof(double*)*MAX_SET_COLS);//save old adresses

if (valid==false)//set completely new -- should not happen, because we are talking about undo and a deleted set has been existend in the past
{
    cout << "invalid set: G" << gno << ".S" << setno << ", unable to restore yet" << endl;
}
else//set exists or has existed (in any way: memory already allocated)
{
    if (what!=UNDO_DATA)//data only means: do not reinstall appearance
    memcpy(g[gno].p+setno,pa,sizeof(plotarr));//set appearance

    //do not override data (at first) --> copy old addresses back to set
    g[gno].p[setno].data.s=sav_s;
    memcpy(g[gno].p[setno].data.ex,sav_ex,sizeof(double*)*MAX_SET_COLS);
    g[gno].p[setno].type=sav_type;
    g[gno].p[setno].data.len=sav_length;

    if (what!=UNDO_APPEARANCE)//appearance only means: do not reinstall data
    {//really reinstall set data
    //g[gno].p[setno].data.len=pa->data.len;//in case no copy of appearance has been done
        set_dataset_type(gno,setno,pa->type);//set new type
        setlength(gno,setno,pa->data.len);//set new length
    for (i=0;i<cols;i++)//reinstall data
    memcpy(g[gno].p[setno].data.ex[i],pa->data.ex[i],pa->data.len*sizeof(double));

        if (pa->data.s!=NULL)//copy strings
        {
        ///g[gno].p[setno].data.s = (char**)xrealloc(g[gno].p[setno].data.s , (pa->data.len)*sizeof(char*));
            for (i=0;i<pa->data.len;i++)
            {
            ///g[gno].p[setno].data.s[i]=(char*)xrealloc(g[gno].p[setno].data.s[i], (strlen(pa->data.s[i]) + 1)*sizeof(char));
            ///strcpy(g[gno].p[setno].data.s[i],pa->data.s[i]);
            g[gno].p[setno].data.s[i]=copy_string(g[gno].p[setno].data.s[i],pa->data.s[i]);
            }
        }
    }//end UNDO_APPEARANCE
}
}

void copyGraph(int gno,graph * gr,int what)//we assume, that gr has ben allocated
{
    if (what==UNDO_COMPLETE || what&UNDO_APPEARANCE)
    {
    memcpy(gr,g+gno,sizeof(graph));//copy set appearance
    gr->labs.title.s=copy_string(NULL,g[gno].labs.title.s);
    gr->labs.stitle.s=copy_string(NULL,g[gno].labs.stitle.s);
    }
if (what==UNDO_COMPLETE || what&UNDO_AXIS)
{
for (int j=0;j<MAXAXES;j++)//copy tickmarks
gr->t[j]=copy_graph_tickmarks(g[gno].t[j]);
}
    if (what&UNDO_DATA || what==UNDO_COMPLETE)
    {
    gr->p=new plotarr[g[gno].maxplot];//generate enough space for the sets
    for (int j=0;j<g[gno].maxplot;j++)//go through all sets
    copySet(gno,j,gr->p+j,what);
    }
}

void deleteSavedGraph(graph * gr,int what)
{
    if (gr==NULL) return;
    if (what&UNDO_DATA || what==UNDO_COMPLETE)
    {
        dont_delete_saved_set_memory=true;
        for (int j=0;j<gr->maxplot;j++)//go through all sets
        {
        deleteSavedSet(gr->p+j,what);
        }
        dont_delete_saved_set_memory=false;
    delete[] gr->p;//delete plotarr
    }
        if (what&UNDO_APPEARANCE || what==UNDO_COMPLETE)
        {
        XCFREE(gr->labs.title.s);
        XCFREE(gr->labs.stitle.s);
        }
    if (what==UNDO_COMPLETE || what&UNDO_AXIS)
    {
        for (int j = 0; j < MAXAXES; j++)
        free_graph_tickmarks(gr->t[j]);
    }
    delete gr;//delete all memory
    gr=NULL;
}

void reinstallGraph(int gno,graph * gr,int what)//you have to make sure that gno is valid / exists
{
    if (gr==NULL) return;
int valid=is_valid_gno(gno);
///int activ=is_active_gno()
int stat=graph_allocate(gno);
if (valid==false)
{
///cout << "gno=" << gno << " invalid" << endl;
    if (stat==RETURN_FAILURE)
    {
    cout << "Error! Could not allocate new grpah!" << endl;
    return;
    }
}

    /*
    if (what==UNDO_COMPLETE || what==UNDO_AXIS)
    for (int j=0;j<MAXAXES;j++)//copy tickmarks
    free_graph_tickmarks(g[gno].t[j]);
    */

tickmarks *t[MAXAXES];
memcpy(t,g[gno].t,sizeof(tickmarks*)*MAXAXES);//save tickmarks adresses
plotarr * pl=g[gno].p;//save adress of plotarrays
char * s1=g[gno].labs.title.s;
char * s2=g[gno].labs.stitle.s;
if (what&UNDO_DATA || what==UNDO_COMPLETE)//clean up old sets
{
kill_all_sets(gno);
pl=g[gno].p;
}

    if (what==UNDO_COMPLETE || what&UNDO_APPEARANCE)
    {
    memcpy(g+gno,gr,sizeof(graph));//copy graph appearance
    g[gno].labs.title.s=copy_string(s1,gr->labs.title.s);
    g[gno].labs.stitle.s=copy_string(s2,gr->labs.stitle.s);
    }
if (what&UNDO_APPEARANCE)//reinstall tickmark_adresses, because appearance does not mean tickmarks
{
memcpy(g[gno].t,t,sizeof(tickmarks*)*MAXAXES);
g[gno].p=pl;//reinstall data-pointers
}
    if (what==UNDO_COMPLETE || what&UNDO_AXIS)
    {
        for (int j=0;j<MAXAXES;j++)//copy tickmarks
        {
        free_graph_tickmarks(t[j]);
        g[gno].t[j]=copy_graph_tickmarks(gr->t[j]);
        }
    }
if (what&UNDO_DATA || what==UNDO_COMPLETE)
{
/*g[gno].p = (plotarr *)xrealloc(pl,gr->maxplot * sizeof(plotarr));
g[gno].maxplot=gr->maxplot;*/
g[gno].maxplot=0;
g[gno].p=pl;
//kill_all_sets(gno);
realloc_graph_plots(gno,gr->maxplot);
    for (int j=0;j<gr->maxplot;j++)//go through all sets
    {
    reinstallSet(gno,j,gr->p+j,what);
    }
}
}

int get_inverse_action(int i)
{
static int inv;
if (abs(i)<100)//possible inverse
{
inv=-i;
}
else
{
inv=i;//inverse is the same action (previous / after)
}
return inv;
}

bool isNodeActive(int i)
{
return Node[i].active;
}

void RestoreNode(int i)
{
Node[index_translator(i)].reStoreData();
}

void updateUndoList(void)
{
if (FormUndoList!=NULL)
FormUndoList->init();
}

void addAditionalDescriptionToLastNode(int type,QString description,QString additional,int mult)
{
undo_node * nn=getPreviousNode();
    if (type!=-1)//we do not set invalid types!
    nn->type=type;
    if (!description.isEmpty())
    nn->Description=description;
    if (!additional.isEmpty())
    nn->AdditionalDescription=additional;
    if (mult!=-1)
    nn->multiple=(bool)mult;
updateUndoList();
}

void SaveSetStatesPrevious(int len,int * gnos,int * snos,int what)
{
if (undo_active==false) return;
///cout << "begin save set states previous" << endl;
    if (prev_sets!=NULL)
    {
    ///cout << "deleting old sets" << endl;
    for (int i=0;i<saved_prev_sets;i++)
    deleteSavedSet(prev_sets[i],what);
    ///cout << "realy deleting now" << endl;
    delete[] prev_sets;
    prev_sets=NULL;
    }
if (len>0)
{
    prev_sets=new plotarr*[len];
    ///cout << "new space for new sets allocated" << endl;
    if (old_idata[0]!=NULL)
    {
    delete[] old_idata[0];
    old_idata[0]=NULL;
    }
    if (old_idata[1]!=NULL)
    {
    delete[] old_idata[1];
    old_idata[1]=NULL;
    }
    ///cout << "old_idata deleted" << endl;
    old_idata[0]=new int[len];
    old_idata[1]=new int[len];
    memcpy(old_idata[0],gnos,len*sizeof(int));
    memcpy(old_idata[1],snos,len*sizeof(int));
    ///cout << "ids coppied begin copying of plotarrs" << endl;
        for (int i=0;i<len;i++)
        {
        prev_sets[i]=new plotarr;
        copySet(gnos[i],snos[i],prev_sets[i],what);
        }
    ///cout << "old sets copied" << endl;
}
    saved_prev_sets=len;
}

void SetsModified(int len,int * gnos,int * snos,int what)
{
if (undo_active==false) return;
undo_node * nn=getNextNode();
///cout << "sets modified begins" << endl;
nn->clearContents();
nn->active=true;
if (what==UNDO_APPEARANCE)
{
nn->type=UNDO_TYPE_SET_APPEARANCE;
nn->Description=QObject::tr("Set appearance changed");
}
else
{
nn->type=UNDO_TYPE_SET_MODIFIED;
nn->Description=QObject::tr("Set modified");
}
    if (len>1 || saved_prev_sets>1)
    {
    nn->multiple=true;
    }
    else
    {
    nn->multiple=false;
    sprintf(dummy,"[G%d.S%d]",gnos[0],snos[0]);
    nn->Description+=QString(dummy);
    }
nn->origin[0]=gnos[0];//no one is actually using this!
nn->origin[1]=saved_prev_sets;//number of sets in first go
nn->origin[2]=len;//number of sets after modification
nn->origin[3]=what;
nn->id[0]=new int[len+saved_prev_sets];//graph-ids
nn->id[1]=new int[len+saved_prev_sets];//set-ids
memcpy(nn->id[0],old_idata[0],saved_prev_sets*sizeof(int));
memcpy(nn->id[1],old_idata[1],saved_prev_sets*sizeof(int));
if (old_idata[0]!=NULL)
{
delete[] old_idata[0];
old_idata[0]=NULL;
}
if (old_idata[1]!=NULL)
{
delete[] old_idata[1];
old_idata[1]=NULL;
}
memcpy(nn->id[0]+saved_prev_sets,gnos,len*sizeof(int));
memcpy(nn->id[1]+saved_prev_sets,snos,len*sizeof(int));
plotarr ** pa=new plotarr*[len+saved_prev_sets];//save old and new sets
///cout << "saving old sets" << endl;
    for (int i=0;i<saved_prev_sets;i++)
    {
    pa[i]=prev_sets[i];//previous//adresses are enough
    prev_sets[i]=NULL;//don't delete this next time!
    }
///cout << "saving new sets" << endl;
    for (int i=saved_prev_sets;i<saved_prev_sets+len;i++)
    {
    pa[i]=new plotarr;//allocate memory
    copySet(gnos[i-saved_prev_sets],snos[i-saved_prev_sets],pa[i],what);//after
    }
///cout << "finished saving sets" << endl;
nn->data=(void*)pa;
NextNode();//finish writing and prepare the next node
updateUndoList();
}

void SaveGraphStatesPrevious(int len,int * gnos,int what)
{
if (undo_active==false) return;
if (prev_graphs!=NULL)
{
for (int i=0;i<saved_prev_graphs;i++)
deleteSavedGraph(prev_graphs[i],what);
delete[] prev_graphs;
prev_graphs=NULL;
}
prev_graphs=new graph*[len];
old_idata[0]=new int[len];
memcpy(old_idata[0],gnos,len*sizeof(int));
saved_prev_graphs=len;
    for (int i=0;i<len;i++)
    {
    prev_graphs[i]=new graph;
    copyGraph(gnos[i],prev_graphs[i],what);
    }
}

void GraphsModified(int len,int * gnos,int what)
{
if (undo_active==false) return;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_GRAPH_APPEARANCE;
nn->Description=QObject::tr("Graph appearance changed");
    if (len>1)
    {
    nn->multiple=true;
    }
    else
    {
    nn->multiple=false;
    sprintf(dummy,"[G%d]",gnos[0]);
    nn->Description+=QString(dummy);
    }
nn->origin[0]=gnos[0];
nn->origin[1]=saved_prev_graphs;
nn->origin[2]=len;
nn->origin[3]=what;
nn->id[0]=new int[len+saved_prev_graphs];
memcpy(nn->id[0],old_idata[0],saved_prev_graphs*sizeof(int));
old_idata[0]=NULL;
memcpy(nn->id[0]+saved_prev_graphs,gnos,len*sizeof(int));
graph ** pa=new graph*[len+saved_prev_graphs];//'*2' because: previous and after!
    for (int i=0;i<saved_prev_graphs;i++)
    {
    pa[i]=prev_graphs[i];//previous
    prev_graphs[i]=NULL;//don't delete this next time!
    }
    for (int i=saved_prev_graphs;i<saved_prev_graphs+len;i++)
    {
    pa[i]=new graph;//allocate new memory
    copyGraph(gnos[i-saved_prev_graphs],pa[i],what);//after
    }
nn->data=(void*)pa;
NextNode();//finish writing and prepare the next node
updateUndoList();
}

void SaveTickmarksStatesPrevious(int axis_start,int axis_stop,int graph_start,int graph_stop)
{
if (undo_active==false) return;
int counter=(graph_stop-graph_start+1)*(axis_stop-axis_start+1);
if (counter<1) return;
world w;
for (int i=0;i<4;i++)
{
if (old_ddata[i]!=NULL)
{
delete[] old_ddata[i];
old_ddata[i]=NULL;
}
if (old_idata[i]!=NULL)
{
delete[] old_idata[i];
old_idata[i]=NULL;
}
old_ddata[i]=new double[counter];
old_idata[i]=new int[counter];
}
    if (prev_tickmarks!=NULL)
    {
    for (int i=0;i<saved_prev_tickmarks;i++)
    free_graph_tickmarks(prev_tickmarks[i]);
    delete[] prev_tickmarks;
    prev_tickmarks=NULL;
    }
    saved_prev_tickmarks=counter;
    prev_tickmarks=new tickmarks*[counter];//only enough space for pointers
    counter=0;
    for (int i=graph_start;i<=graph_stop;i++)
    {
        get_graph_world(i, &w);
        for (int j=axis_start;j<=axis_stop;j++)
        {
        prev_tickmarks[counter]=copy_graph_tickmarks(g[i].t[j]);//deep copy of all contents
        old_ddata[0][counter]=w.xg1;
        old_ddata[1][counter]=w.xg2;
        old_ddata[2][counter]=w.yg1;
        old_ddata[3][counter]=w.yg2;
        old_idata[0][counter]=get_graph_xscale(i);
        old_idata[1][counter]=get_graph_yscale(i);
        old_idata[2][counter]=is_graph_xinvert(i);
        old_idata[3][counter]=is_graph_yinvert(i);
        counter++;
        }
    }
}

void TickmarksModified(int axis_start,int axis_stop,int graph_start,int graph_stop)
{
if (undo_active==false) return;
int counter=(graph_stop-graph_start+1)*(axis_stop-axis_start+1);
if (counter<1) return;
world w;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_AXIS_PROPERTIES;
nn->Description=QObject::tr("Axis properties changed");
    if (counter>1)
    {
    nn->multiple=true;
    }
    else
    {
    nn->multiple=false;
    sprintf(dummy,"[G%d.",graph_start);
    nn->Description+=QString(dummy);
        if (axis_start==0)
        nn->Description+=QObject::tr("X-axis");
        else if (axis_start==1)
        nn->Description+=QObject::tr("Y-axis");
        else if (axis_start==2)
        nn->Description+=QObject::tr("Alt-X-axis");
        else if (axis_start==3)
        nn->Description+=QObject::tr("Alt-Y-axis");
    nn->Description+=QString("]");
    }
nn->origin[2]=counter;//important
nn->origin[0]=axis_start;
nn->origin[1]=axis_stop;
nn->origin[3]=graph_start;
nn->id[0]=new int[4];//save axis numbers concerned
nn->id[0][0]=axis_start;
nn->id[0][1]=axis_stop;
nn->id[0][2]=graph_start;
nn->id[0][3]=graph_stop;
nn->id[1]=new int[4*(counter*2)+4];//4 integer entries per counter (xscale,yscale,xinvert,yinvert)
for (int i=0;i<4;i++)
nn->ddata[i]=new double[counter*2];
tickmarks ** t_tickmarks=new tickmarks*[counter*2];//'*2' because: previous and after
counter=0;
for (int i=graph_start;i<=graph_stop;i++)
{
    for (int j=axis_start;j<=axis_stop;j++)
    {
    t_tickmarks[counter]=prev_tickmarks[counter];//copy pointer only
    prev_tickmarks[counter]=NULL;//do not delete this next time!
        for (int k=0;k<4;k++)
        {
        nn->ddata[k][counter]=old_ddata[k][counter];
        nn->id[1][4*counter+k]=old_idata[k][counter];
        }
    counter++;
    }//save previous states
}
delete[] prev_tickmarks;
prev_tickmarks=NULL;
saved_prev_tickmarks=0;
    for (int i=graph_start;i<=graph_stop;i++)
    {
        get_graph_world(i, &w);
        for (int j=axis_start;j<=axis_stop;j++)
        {
        t_tickmarks[counter]=copy_graph_tickmarks(g[i].t[j]);//copy new states
        nn->ddata[0][counter]=w.xg1;
        nn->ddata[1][counter]=w.xg2;
        nn->ddata[2][counter]=w.yg1;
        nn->ddata[3][counter]=w.yg2;
        nn->id[1][4*counter+0]=get_graph_xscale(i);
        nn->id[1][4*counter+1]=get_graph_yscale(i);
        nn->id[1][4*counter+2]=is_graph_xinvert(i);
        nn->id[1][4*counter+3]=is_graph_yinvert(i);
        counter++;
        }
    }
nn->data=(void*)t_tickmarks;
NextNode();//finish writing and prepare the next node
    for (int i=0;i<4;i++)
    {
    delete[] old_ddata[i];
    delete[] old_idata[i];
    old_ddata[i]=NULL;
    old_idata[i]=NULL;
    }
}

void SavePlotAppearance(void)
{
if (undo_active==false) return;
old_data[0]=draw_props.bgcolor;
old_data[1]=draw_props.bgfilled;
if (old_tmstmp==NULL) old_tmstmp=new plotstr;
memcpy(old_tmstmp,&timestamp,sizeof(plotstr));//remember: this will copy the adress of the string as well --> string should not be made undone!
}

void PlotAppearanceModified(void)
{
if (undo_active==false) return;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_PLOT_APPEARANCE;
nn->Description=QObject::tr("Plot appearance changed");
nn->multiple=false;
nn->origin[0]=old_data[0];
nn->origin[1]=old_data[1];
nn->origin[2]=draw_props.bgcolor;
nn->origin[3]=draw_props.bgfilled;
plotstr ** tmstmp=new plotstr*[2];
tmstmp[0]=old_tmstmp;
tmstmp[1]=new plotstr;
old_tmstmp=NULL;
memcpy(tmstmp[1],&timestamp,sizeof(plotstr));//remember: this will copy the adress of the string as well --> string should not be made undone!
nn->data=(void*)tmstmp;
NextNode();//finish writing and prepare the next node
updateUndoList();
}

void SaveObjectData(int id,int type)
{
if (undo_active==false) return;
switch (type)
{
case OBJECT_BOX:
    if (sav_bt!=NULL) delete sav_bt;
    sav_bt=new boxtype;
    memcpy(sav_bt,boxes+id,sizeof(boxtype));
break;
case OBJECT_LINE:
    if (sav_lt!=NULL) delete sav_lt;
    sav_lt=new linetype;
    memcpy(sav_lt,lines+id,sizeof(linetype));
break;
case OBJECT_ELLIPSE:
    if (sav_et!=NULL) delete sav_et;
    sav_et=new ellipsetype;
    memcpy(sav_et,ellip+id,sizeof(ellipsetype));
break;
case OBJECT_STRING:
    if (sav_st!=NULL) delete sav_st;
    sav_st=new plotstr;
    memcpy(sav_st,pstr+id,sizeof(plotstr));//copy plotstr
        if (sav_st->s!=NULL)//copy the actual string/text
        {
        sav_st->s=new char[strlen(pstr[id].s)+1];
        strcpy(sav_st->s,pstr[id].s);
        }
break;
}
}

void ObjectDataModified(int id,int type)
{
if (undo_active==false) return;
undo_node * nn=getNextNode();
linetype ** lt;
boxtype ** bt;
ellipsetype ** et;
plotstr ** st;
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_OBJECT_MODIFIED;
nn->Description=QObject::tr("Object edited");
nn->multiple=false;
nn->origin[0]=id;
nn->origin[1]=type;
nn->origin[2]=1;
nn->id[0]=new int[2];
nn->id[0][0]=nn->id[0][1]=id;
switch (type)
{
case OBJECT_BOX:
    bt=new boxtype*[2];//'2' = previous + after
    nn->Description=QObject::tr("Box edited");
    bt[0]=sav_bt;
    sav_bt=NULL;
    bt[1]=new boxtype;
    memcpy(bt[1],boxes+id,sizeof(boxtype));
    nn->data=(void*)bt;
break;
case OBJECT_LINE:
    lt=new linetype*[2];
    nn->Description=QObject::tr("Line edited");
    lt[0]=sav_lt;
    sav_lt=NULL;
    lt[1]=new linetype;
    memcpy(lt[1],lines+id,sizeof(linetype));
    nn->data=(void*)lt;
break;
case OBJECT_ELLIPSE:
    et=new ellipsetype*[2];
    nn->Description=QObject::tr("Ellipse edited");
    et[0]=sav_et;
    sav_et=NULL;
    et[1]=new ellipsetype;
    memcpy(et[1],ellip+id,sizeof(ellipsetype));
    nn->data=(void*)et;
break;
case OBJECT_STRING:
    st=new plotstr*[2];
    nn->Description=QObject::tr("String edited");   
    if (sav_st->s[0]=='\0')
    {
    nn->type=UNDO_TYPE_OBJECT_INSERTED;
    nn->Description=QObject::tr("String inserted");
    st[0]=new plotstr;
    memcpy(st[0],pstr+id,sizeof(plotstr));
        if (st[0]->s!=NULL)
        {
        st[0]->s=new char[strlen(pstr[id].s)+1];
        strcpy(st[0]->s,pstr[id].s);
        }
    }
    else
    {
    st[0]=sav_st;
    sav_st=NULL;
    }
    st[1]=new plotstr;
    memcpy(st[1],pstr+id,sizeof(plotstr));
        if (st[1]->s!=NULL)
        {
        st[1]->s=new char[strlen(pstr[id].s)+1];
        strcpy(st[1]->s,pstr[id].s);
        }
    nn->data=(void*)st;
break;
}
NextNode();//finish writing and prepare the next node
updateUndoList();
}

void SaveHotlinkData(int len,int * gnos,int * snos)
{
if (undo_active==false || len<=0) return;
if (old_strings!=NULL)
{
    for (int i=0;i<number_of_old_strings;i++)
    {
        if (old_strings[i]!=NULL)
        {
        delete[] old_strings[i];
        old_strings[i]=NULL;
        }
    }
delete[] old_strings;
old_strings=NULL;
}
if (old_idata[0]!=NULL)
{
delete[] old_idata[0];
old_idata[0]=NULL;
}
if (old_idata[1]!=NULL)
{
delete[] old_idata[1];
old_idata[1]=NULL;
}
number_of_old_strings=len;
old_strings=new char *[len];
old_idata[0]=new int[len];
old_idata[1]=new int[len];
    for (int i=0;i<number_of_old_strings;i++)
    {
    old_idata[0][i]=g[gnos[i]].p[snos[i]].hotlink;
        if (g[gnos[i]].p[snos[i]].hotfile!=NULL && old_idata[0][i])
        {
        old_idata[1][i]=g[gnos[i]].p[snos[i]].hotsrc;
        old_strings[i]=new char[strlen(g[gnos[i]].p[snos[i]].hotfile)+2];
        strcpy(old_strings[i],g[gnos[i]].p[snos[i]].hotfile);
        }
        else
        {
        old_idata[1][i]=0;
        old_strings[i]=NULL;
        }
    }
}

void HotlinkModified(int len,int * gnos,int * snos)
{
if (undo_active==false || len<=0) return;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_HOTLINK;
nn->Description=QObject::tr("Hotlink modified");
if (len==1)
{
sprintf(dummy,"[G%d.S%d]",gnos[0],snos[0]);
nn->Description+=QString(dummy);
nn->multiple=false;
}
else
{
nn->multiple=true;
}
nn->origin[0]=0;
nn->origin[1]=0;
nn->origin[2]=len;
nn->origin[3]=0;
nn->id[0]=new int[2*len];
nn->id[1]=new int[2*len];
nn->id[2]=new int[len];
nn->id[3]=new int[len];
memcpy(nn->id[0],old_idata[0],sizeof(int)*len);//old states --> hotlink
memcpy(nn->id[1],old_idata[1],sizeof(int)*len);//old_states --> hotsrc
memcpy(nn->id[2],gnos,sizeof(int)*len);//gnos
memcpy(nn->id[3],snos,sizeof(int)*len);//snos
delete[] old_idata[0];
delete[] old_idata[1];
old_idata[0]=old_idata[1]=NULL;
char ** fnames=new char*[2*len];
for (int i=0;i<len;i++)//old fnames
{
fnames[i]=old_strings[i];
old_strings[i]=NULL;
}
for (int i=0;i<len;i++)//new states
{
nn->id[0][i+len]=g[gnos[i]].p[snos[i]].hotlink;
    if (nn->id[0][i+len] && g[gnos[i]].p[snos[i]].hotfile!=NULL)
    {
    nn->id[1][i+len]=g[gnos[i]].p[snos[i]].hotsrc;
    fnames[i+len]=new char[strlen(g[gnos[i]].p[snos[i]].hotfile)+2];
    strcpy(fnames[i+len],g[gnos[i]].p[snos[i]].hotfile);
    }
    else
    {
    nn->id[1][i+len]=0;
    fnames[i+len]=NULL;
    }
}
nn->data=(void*)fnames;
NextNode();//finish writing and prepare the next node
updateUndoList();
}

void SaveLocatorFixPoint(int gno)
{
GLocator locator;
if (undo_active==false || get_graph_locator(gno, &locator) != RETURN_SUCCESS) return;
GLocator *loc2=new GLocator;
memcpy(loc2,&locator,sizeof(GLocator));
old_general_data=(void*)loc2;
}

void LocatorFixPointModified(int gno)
{
GLocator locator;
if (undo_active==false || get_graph_locator(gno, &locator) != RETURN_SUCCESS) return;
GLocator ** loc2=new GLocator*[2];
loc2[1]=new GLocator;
memcpy(loc2[1],&locator,sizeof(GLocator));
loc2[0]=(GLocator*)old_general_data;
old_general_data=NULL;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_LOCATOR_FIXPOINT;
nn->Description=QObject::tr("Locator fixpoint modified");
sprintf(dummy,"[G%d]",gno);
nn->Description+=QString(dummy);
nn->multiple=false;
nn->origin[0]=gno;
nn->origin[1]=0;
nn->origin[2]=1;
nn->origin[3]=0;
nn->data=(void*)loc2;
//no additional data needed here! No data is destroyed!
NextNode();//finish writing and prepare the next node
updateUndoList();

}

void UndoSwapGraphs(int g1,int g2)
{
if (undo_active==false) return;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_GRAPH_SWAP;
nn->Description=QObject::tr("Graphs swapped");
sprintf(dummy,"[G%d<->G%d]",g1,g2);
nn->Description+=QString(dummy);
nn->multiple=false;
nn->origin[0]=g1;
nn->origin[1]=UNDO_GRAPH;
nn->origin[2]=1;
nn->origin[3]=g2;
//no additional data needed here! No data is destroyed!
NextNode();//finish writing and prepare the next node
updateUndoList();
}

void UndoSwapSets(int g1,int s1,int g2,int s2)
{
if (undo_active==false) return;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_SET_SWAP;
nn->Description=QObject::tr("Sets swapped");
sprintf(dummy,"[G%d.S%d<->G%d.S%d]",g1,s1,g2,s2);
nn->Description+=QString(dummy);
nn->multiple=false;
//just use the available space --> no need to allocate more space
nn->origin[0]=g1;
nn->origin[1]=s1;
nn->origin[2]=g2;
nn->origin[3]=s2;
//no additional data needed here! No data is destroyed!
NextNode();//finish writing and prepare the next node
updateUndoList();
}

void SetsAboutToBePacked(int gno)
{
if (undo_active==false) return;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_PACK_SETS;
nn->Description=QObject::tr("Sets packed");
sprintf(dummy,"[G%d]",gno);
nn->Description+=QString(dummy);
nn->multiple=false;
nn->origin[0]=gno;
nn->origin[1]=UNDO_SET;
nn->origin[2]=g[gno].maxplot;
nn->id[0]=new int[nn->origin[2]];
//All sets have to bee saved, because the inactive sets are lost
plotarr ** pa=new plotarr*[nn->origin[2]];
    for (int i=0;i<nn->origin[2];i++)
    {
    nn->id[0][i]=i;
    pa[i]=new plotarr;
    copySet(gno,i,pa[i],UNDO_COMPLETE);
    }
nn->data=(void*)pa;
NextNode();//finish writing and prepare the next node
updateUndoList();
}

void ViewportChanged(int gno,view v)
{
if (undo_active==false || !is_valid_gno(gno)) return;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_VIEWPORT_CHANGED;
nn->Description=QObject::tr("Graph viewport changed");
sprintf(dummy,"[G%d]",gno);
nn->Description+=QString(dummy);
nn->multiple=false;
nn->origin[0]=gno;
nn->origin[2]=1;
double * views=new double[8];
view old_v;
get_graph_viewport(gno,&old_v);
views[0]=old_v.xv1;
views[1]=old_v.xv2;
views[2]=old_v.yv1;
views[3]=old_v.yv2;
views[4]=v.xv1;
views[5]=v.xv2;
views[6]=v.yv1;
views[7]=v.yv2;
nn->data=(void*)views;
NextNode();//finish writing and prepare the next node
updateUndoList();
}

void copy_Datapoint(int gno,int sno,int nr,Datapoint * p)
{
static int ncols;
static Datapoint p2;
    if (!is_valid_setno(gno,sno) || nr>=getsetlength(gno,sno) || nr<0)
    {
    p=NULL;
    return;
    }
    get_datapoint(gno,sno,nr,&ncols,&p2);
for (int i=0;i<ncols;i++)
p->ex[i]=p2.ex[i];
    if (p2.s!=NULL)
    p->s=copy_string(NULL,p2.s);
    else
    p->s=NULL;
}

void DataPointEdited(int gno,int sno,int * nrs,int len,Datapoint * p,int type)
{
if (undo_active==false || !is_valid_gno(gno)) return;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_DATAPOINT_EDIT;
nn->origin[0]=gno;
nn->origin[1]=sno;
nn->origin[2]=len;
nn->origin[3]=type;
nn->id[0]=new int[len];
memcpy(nn->id[0],nrs,sizeof(int)*len);
Datapoint * ps=new Datapoint[2*len];
    for (int i=0;i<len;i++)
    {
    copy_Datapoint(gno,sno,nrs[i],ps+i);//save old/new datapoints in first len Datapoints
    zero_datapoint(ps+i+len);//initialize the second half of the memory
    }
    if (type==0)
    {
    nn->Description=QObject::tr("New Datapoint");
    }
    else if (type==1)
    {
    nn->Description=QObject::tr("Datapoint deleted");
    }
    else
    {
    nn->Description=QObject::tr("Datapoint modified");
        for (int i=0;i<len;i++)
        {
        memcpy(ps+i+len,p+i,sizeof(Datapoint));
        if (p->s!=NULL)
        ps[i+len].s=copy_string(NULL,p[i].s);
        }
    }
nn->data=(void*)ps;
if (len==1)
{
nn->multiple=false;
sprintf(dummy,"[G%d.S%d(%d)]",gno,sno,nrs[0]);
nn->Description+=QString(dummy);
}
else
{
nn->multiple=true;
}
NextNode();//finish writing and prepare the next node
updateUndoList();
}

void CopyRegion(region * from,region * to)
{
XCFREE(to->x);
XCFREE(to->y);
memcpy(to,from,sizeof(region));
    if (from->x!=NULL)
    {
    to->x=(double*)xcalloc(to->n, sizeof(double));
    memcpy(to->x,from->x,sizeof(double)*to->n);
    }
    if (from->y!=NULL)
    {
    to->y=(double*)xcalloc(to->n, sizeof(double));
    memcpy(to->y,from->y,sizeof(double)*to->n);
    }
}

void DeleteRegion(region * regi)
{
if (regi==NULL) return;
XCFREE(regi->x);
XCFREE(regi->y);
}

void RegionModified(int nr,region * newregion,int type)
{
if (undo_active==false || nr>MAXREGION || nr<0) return;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_REGION;
nn->origin[0]=nr;
nn->origin[1]=nr;
nn->origin[2]=1;
nn->origin[3]=type;
region * ps=NULL;
sprintf(dummy," %d ",nr);
    if (type==0)//modified
    {
    ps=new region[2];
    ps[1].x=ps[1].y=ps[0].x=ps[0].y=NULL;
    //CopyRegion(rg+nr,ps);//old --> not good!
    CopyRegion(&undo_region,ps);
    CopyRegion(newregion,ps+1);//new
    nn->Description=QObject::tr("Region")+QString(dummy)+QObject::tr("modified");
    }
    else if (type==1)//killed --> save old only
    {
        if (nr<MAXREGION)
        {
        ps=new region[2];
        ps[1].x=ps[1].y=ps[0].x=ps[0].y=NULL;
        CopyRegion(rg+nr,ps);//old
        nn->Description=QString("Region")+QString(dummy)+QObject::tr("cleared");
        }
        else//all sets
        {
        nn->origin[2]=MAXREGION;
        ps=new region[MAXREGION];
        for (int i=0;i<MAXREGION;i++)
        {
        ps[i].x=ps[i].y=NULL;
        CopyRegion(rg+i,ps+i);
        }
        nn->Description=QString("All Regions cleared");
        }
    }
nn->data=(void*)ps;
NextNode();//finish writing and prepare the next node
updateUndoList();
}

void SaveFitSettings(void)
{
    if (undo_active==false) return;
    if (prev_sets!=NULL)
    {
    delete[] prev_sets;
    prev_sets=NULL;
    }
get_fit_settings(&old_fit_settings);
    if (old_fit_settings.dest_sets>=0)
    {
    saved_prev_sets=1;
    prev_sets=new plotarr*[1];
    prev_sets[0]=new plotarr;
    copySet(old_fit_settings.dest_gno,old_fit_settings.dest_sets,prev_sets[0],UNDO_COMPLETE);
    }
    else
    {
    prev_sets=NULL;
    saved_prev_sets=0;
    }
}

void SaveFitAfter(void)
{
if (undo_active==false) return;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_FIT;
nn->origin[0]=1;
nn->origin[1]=1;
nn->origin[2]=2;//number of saved sets -- one may be NULL
nn->origin[3]=1;
nn->p=new plotarr*[2];
struct all_fit_settings * ps=new struct all_fit_settings[2];//old / new -- before / after fit
//memcpy(ps,&old_fit_settings,sizeof(struct all_fit_settings));//copy previous state
ps[0]=old_fit_settings;
for (int i=0;i<11;i++)
{
ps[0].value[i]=old_fit_settings.value[i];
ps[0].lower[i]=old_fit_settings.lower[i];
ps[0].upper[i]=old_fit_settings.upper[i];
ps[0].bound[i]=old_fit_settings.bound[i];
}
get_fit_settings(ps+1);
if (saved_prev_sets>0)//there has been a set before
{
nn->p[0]=prev_sets[0];
prev_sets[0]=NULL;
saved_prev_sets=0;
}
else
{
nn->p[0]=NULL;//no previous set
}
nn->p[1]=new plotarr;
copySet(ps[1].dest_gno,ps[1].dest_sets,nn->p[1],UNDO_COMPLETE);//save set after fit
sprintf(dummy," [G%d.S%d]",old_fit_settings.src_gno,old_fit_settings.src_sets);
nn->Description=QObject::tr("A fit to")+QString(dummy);
nn->AdditionalDescription=old_fit_settings.formula;
nn->data=(void*)ps;
NextNode();//finish writing and prepare the next node
updateUndoList();
}

void ChangeSetOrdering(int gno,int setno,int type)
{
if (undo_active==false) return;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_NEW_ORDERING;
nn->origin[0]=gno;
nn->origin[1]=setno;
nn->origin[2]=1;
nn->origin[3]=type;
sprintf(dummy," [G%d.S%d] ",gno,setno);
    if (type==PUSH_SET_TOFRONT)
    nn->Description=QObject::tr("Set")+QString(dummy)+QObject::tr("brought to front");
    else
    nn->Description=QObject::tr("Set")+QString(dummy)+QObject::tr("sent to back");
nn->data=NULL;
NextNode();//finish writing and prepare the next node
updateUndoList();
}

void ShowHideSets(int len,int * gnos,int * sets)
{
if (undo_active==false) return;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_SHOW_HIDE;
nn->origin[0]=0;
nn->origin[1]=0;
nn->origin[2]=len;
nn->origin[3]=UNDO_SET;
nn->id[0]=new int[len+1];
nn->id[1]=new int[len+1];
memcpy(nn->id[0],gnos,len*sizeof(int));
memcpy(nn->id[1],sets,len*sizeof(int));
nn->Description=QObject::tr("Set visibility changed");
sprintf(dummy,"[G%d.S%d]",gnos[0],sets[0]);
    if (len==1)
    {
    nn->multiple=false;
    nn->Description+=QString(dummy);
    }
    else
    {
    nn->multiple=true;
    }
nn->data=NULL;
NextNode();//finish writing and prepare the next node
updateUndoList();
}

void ShowHideGraphs(int len,int * gnos)
{
if (undo_active==false) return;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_SHOW_HIDE;
nn->origin[0]=0;
nn->origin[1]=0;
nn->origin[2]=len;
nn->origin[3]=UNDO_GRAPH;
nn->id[0]=new int[len+1];
memcpy(nn->id[0],gnos,len*sizeof(int));
nn->Description=QObject::tr("Graph visibility changed");
sprintf(dummy,"[G%d]",gnos[0]);
    if (len==1)
    {
    nn->multiple=false;
    nn->Description+=QString(dummy);
    }
    else
    {
    nn->multiple=true;
    }
nn->data=NULL;
NextNode();//finish writing and prepare the next node
updateUndoList();
}

void get_fit_settings(struct all_fit_settings * fit)
{
    if (undo_active==false) return;
    if (FormNonlinCurveFit==NULL) return;
GetSingleListChoice(FormNonlinCurveFit->grpSource->listGraph, &fit->src_gno);
GetSingleListChoice(FormNonlinCurveFit->grpSource->listSet, &fit->src_sets);
fit->dest_sets=-1;
GetSingleListChoice(FormNonlinCurveFit->grpDestination->listGraph, &fit->dest_gno);
GetSingleListChoice(FormNonlinCurveFit->grpDestination->listSet,&fit->dest_sets);
fit->formula=FormNonlinCurveFit->tabMain->ledFormula->text();
fit->Tolerance=FormNonlinCurveFit->tabMain->ledTolerance->text();
fit->iterations=FormNonlinCurveFit->tabMain->selIterations->value();
fit->nr_of_parameters=FormNonlinCurveFit->tabMain->selParameters->currentIndex();
for (int i=0;i<fit->nr_of_parameters;i++)
{
fit->bound[i]=FormNonlinCurveFit->tabMain->spreadParameters->fitlines[i]->chkBonds->isChecked();
fit->lower[i]=FormNonlinCurveFit->tabMain->spreadParameters->fitlines[i]->ledLowerBound->text();
fit->upper[i]=FormNonlinCurveFit->tabMain->spreadParameters->fitlines[i]->ledHighterBound->text();
fit->value[i]=FormNonlinCurveFit->tabMain->spreadParameters->fitlines[i]->ledValue->text();
}
fit->weight_func=FormNonlinCurveFit->tabAdvanced->ledFunction->text();
fit->weights=FormNonlinCurveFit->tabAdvanced->selWeights->currentIndex();
fit->restrictions=FormNonlinCurveFit->tabAdvanced->selRestrictions->currentIndex();
fit->negated=FormNonlinCurveFit->tabAdvanced->chkNegated->isChecked();
fit->load=FormNonlinCurveFit->tabAdvanced->selLoad->currentIndex();
fit->start=FormNonlinCurveFit->tabAdvanced->ledStartLoadAt->text();
fit->stop=FormNonlinCurveFit->tabAdvanced->ledStopLoadAt->text();
fit->nr=FormNonlinCurveFit->tabAdvanced->ledNumberOfPoints->text();
}

void set_fit_settings(struct all_fit_settings * fit)
{
    if (undo_active==false) return;
    if (FormNonlinCurveFit==NULL) return;
FormNonlinCurveFit->grpSource->listGraph->set_graph_number(fit->src_gno,true);
FormNonlinCurveFit->grpSource->set_graph_nr(fit->src_gno);
FormNonlinCurveFit->grpDestination->listGraph->set_graph_number(fit->dest_gno,true);
FormNonlinCurveFit->grpDestination->set_graph_nr(fit->dest_gno);
SelectListChoice(FormNonlinCurveFit->grpSource->listSet,fit->src_sets);
    if (fit->dest_sets==-1)
    FormNonlinCurveFit->grpDestination->listSet->clearSelection();
    else
    SelectListChoice(FormNonlinCurveFit->grpDestination->listSet,fit->dest_sets);
FormNonlinCurveFit->tabMain->ledFormula->setText(fit->formula);
FormNonlinCurveFit->tabMain->ledTolerance->setText(fit->Tolerance);
FormNonlinCurveFit->tabMain->selIterations->setValue(fit->iterations);
FormNonlinCurveFit->tabMain->selParameters->setCurrentIndex(fit->nr_of_parameters);
for (int i=0;i<fit->nr_of_parameters;i++)
{
FormNonlinCurveFit->tabMain->spreadParameters->fitlines[i]->chkBonds->setChecked(fit->bound[i]);
FormNonlinCurveFit->tabMain->spreadParameters->fitlines[i]->ledLowerBound->setText(fit->lower[i]);
FormNonlinCurveFit->tabMain->spreadParameters->fitlines[i]->ledHighterBound->setText(fit->upper[i]);
FormNonlinCurveFit->tabMain->spreadParameters->fitlines[i]->ledValue->setText(fit->value[i]);
}
FormNonlinCurveFit->tabAdvanced->ledFunction->setText(fit->weight_func);
FormNonlinCurveFit->tabAdvanced->selWeights->setCurrentIndex(fit->weights);
FormNonlinCurveFit->tabAdvanced->selRestrictions->setCurrentIndex(fit->restrictions);
FormNonlinCurveFit->tabAdvanced->chkNegated->setChecked(fit->negated);
FormNonlinCurveFit->tabAdvanced->selLoad->setCurrentIndex(fit->load);
FormNonlinCurveFit->tabAdvanced->ledStartLoadAt->setText(fit->start);
FormNonlinCurveFit->tabAdvanced->ledStopLoadAt->setText(fit->stop);
FormNonlinCurveFit->tabAdvanced->ledNumberOfPoints->setText(fit->nr);
}

void SaveDeviceState(int id,bool sync)
{
    if (undo_active==false) return;
    if (saved_entry!=NULL) delete saved_entry;
    saved_entry=new Device_entry;
    saved_entry[0] = get_device_props(id);
    //save viewports if sync is active
    if (sync)
    {
    if (old_idata[0]!=NULL) delete[] old_idata[0];
    old_idata[0]=new int[2*ndevices];
        for (unsigned int i=0;i<ndevices;i++)
        {
        old_idata[0][i]=device_table[i].pg.height;
        old_idata[0][ndevices+i]=device_table[i].pg.width;
        }
    }
}

void DeviceModified(int id,bool sync)
{
if (undo_active==false) return;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_DEVICE_MODIFIED;
nn->origin[0]=sync;
nn->origin[1]=ndevices;
nn->origin[2]=1;
nn->origin[3]=id;
nn->Description=QObject::tr("Device options changed");
sprintf(dummy," (%s)",get_device_name(id));
nn->Description+=QString(dummy);
nn->multiple=false;
Device_entry ** ps=new Device_entry*[2];
ps[0]=saved_entry;
saved_entry=NULL;
ps[1]=new Device_entry;
ps[1][0] = get_device_props(id);
nn->data=(void*)ps;
    if (sync)//save old viewports (new ones don't need to be saved scince they are synced)
    {
    nn->id[0]=old_idata[0];
    old_idata[0]=NULL;
    nn->id[1]=new int[2*ndevices];
    for (unsigned int i=0;i<ndevices;i++)
    {
    nn->id[1][i]=device_table[i].pg.height;
    nn->id[1][ndevices+i]=device_table[i].pg.width;
    }
    }
NextNode();//finish writing and prepare the next node
updateUndoList();
}

int contains(int * list,int len,int number)
{
int ret=-1;
for (int i=0;i<len;i++)
{
    if (list[i]==number)
    {
    ret=i;
    break;
    }
}
return ret;
}

int ContainsGS(int * gnos,int * snos,int len,int gno,int setno)
{
int ret=-1;
for (int i=0;i<len;i++)
{
    if (gnos[i]==gno && snos[i]==setno)
    {
    ret=i;
    break;
    }
}
return ret;
}

int NodeContainsKilledSet(int startnode,bool forward,int gno,int setno)
{
int ret=-1;
int stopnode;
int value;
    if (forward==false)//backwards
    {
            if (used_Nodes<max_node_nr || (used_Nodes==max_node_nr && NodeNr==0))
            {
            stopnode=0;//no wrap around (yet)
            value=-1;
                for (int i=startnode;i>=stopnode;i--)
                {
                value=Node[i].containsSet(gno,setno);
                    if (value!=-1)
                    {
                    ret=i;
                    break;
                    }
                }
            }
            else//with wrap around
            {
            stopnode=NodeNr;
            value=-1;
                if (startnode<NodeNr)
                {
                    for (int i=startnode;i>=0;i--)
                    {
                    value=Node[i].containsSet(gno,setno);
                        if (value!=-1)
                        {
                        ret=i;
                        break;
                        }
                    }
                }
                if (ret==-1)
                {
                    for (int i=max_node_nr-1;i>=stopnode;i--)
                    {
                    value=Node[i].containsSet(gno,setno);
                        if (value!=-1)
                        {
                        ret=i;
                        break;
                        }
                    }
                }
            }

    }
    else//forwards
    {
        if (used_Nodes<max_node_nr || (used_Nodes==max_node_nr && NodeNr==0))
        {
            stopnode=NodeNr>0?NodeNr-1:max_node_nr-1;//no wrap around (yet)
            value=-1;
                for (int i=startnode;i<=stopnode;i++)
                {
                value=Node[i].containsSet(gno,setno);
                    if (value!=-1)
                    {
                    ret=i;
                    break;
                    }
                }
        }
        else
        {//with wraparound
            stopnode=NodeNr-1;
            value=-1;
            if (startnode>=NodeNr)
            {
                for (int i=startnode;i<max_node_nr;i++)
                {
                value=Node[i].containsSet(gno,setno);
                    if (value!=-1)
                    {
                    ret=i;
                    break;
                    }
                }
            }
            if (ret==-1)
            {
                for (int i=0;i<stopnode;i++)
                {
                value=Node[i].containsSet(gno,setno);
                    if (value!=-1)
                    {
                    ret=i;
                    break;
                    }
                }
            }
        }
    }
return ret;
}

void SetImported(int gno,int setno,char * filename,int cursource,int load,int autoscale)
{
if (undo_active==false) return;
undo_node * nn=getNextNode();
nn->clearContents();
nn->active=true;
nn->type=UNDO_TYPE_IMPORT_ASCII;
nn->origin[0]=gno;
nn->origin[1]=setno;
nn->origin[2]=1;
nn->origin[3]=0;
sprintf(dummy,"[G%d.S%d]",gno,setno);
nn->Description=QObject::tr("Import from ASCII --> ")+QString(dummy);
nn->multiple=false;
nn->savedFileName=QString(filename);
nn->id[0]=new int[5];
nn->id[0][0]=gno;
nn->id[0][1]=setno;
nn->id[0][2]=cursource;
nn->id[0][3]=load;
nn->id[0][4]=autoscale;
NextNode();//finish writing and prepare the next node
updateUndoList();
}

void SetImportBlockData(int gno, int setno,char * filename,int source, int nc, int *coli, int scol,int load,int autoscale)
{
if (undo_active==false) return;
SetImported(gno,setno,filename,source,load,autoscale);
undo_node * nn=getPreviousNode();
nn->type=UNDO_TYPE_IMPORT_BLOCK_DATA;
nn->id[1]=new int[2];
nn->id[1][0]=nc;
nn->id[1][1]=scol;
nn->id[2]=new int[nc];
memcpy(nn->id[2],coli,sizeof(int)*nc);
//save old set before Import
if (is_valid_setno(gno,setno))
{
nn->p=new plotarr*[1];
nn->p[0]=new plotarr;
copySet(gno,setno,nn->p[0],UNDO_COMPLETE);
}//else p should be NULL because of the clearContents()-command in the SetImported-function

//writing finish and already everything prepared for the next node
}

void SetNewSetNo(int setno)
{
if (undo_active==false) return;
undo_node * nn=getPreviousNode();
if (nn->type==UNDO_TYPE_IMPORT_BLOCK_DATA)
{
nn->id[0][1]=nn->origin[1]=setno;
sprintf(dummy,"[G%d.S%d]",nn->origin[0],setno);
nn->Description=QObject::tr("BlockData --> ")+QString(dummy);
}
updateUndoList();
}

void SetImportBinaryData(int gno,int setno,char * filename,struct importSettings * set)
{

}

void CopyBinaryImportSettings(struct importSettings * from,struct importSettings * to)
{

}

void DeleteBinaryImportSettings(struct importSettings * set)
{

}

void getHotlinkedSets(int * nr,int ** gnos,int ** snos)
{
*nr=0;
    for (int gno = 0; gno < number_of_graphs(); gno++)
    {
        for (int setno = 0; setno < number_of_sets(gno); setno++)
        {
        if (is_hotlinked(gno, setno) == TRUE) *nr+=1;
        }
    }
delete[] *gnos;
delete[] *snos;
*gnos=new int[*nr+1];
*snos=new int[*nr+1];
*nr=0;
    for (int gno = 0; gno < number_of_graphs(); gno++)
    {
        for (int setno = 0; setno < number_of_sets(gno); setno++)
        {
            if (is_hotlinked(gno, setno) == TRUE)
            {
            *gnos[*nr]=gno;
            *snos[*nr]=setno;
            *nr+=1;
            }
        }
    }
}

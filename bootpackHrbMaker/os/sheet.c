#include "bootpack.h"

struct SHTCTL *shtctl_init(struct MEMMAN* mem_manager, unsigned char* vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL*) memman_alloc_4k(mem_manager, sizeof(struct SHTCTL));
	if(!ctl)
	      return ctl;
	
	ctl->map = (unsigned char*) memman_alloc_4k(mem_manager, xsize*ysize);
	if(!ctl->map)	
	{
	      memman_free_4k(mem_manager, (int)ctl, sizeof(struct SHTCTL));
	      return ctl;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1;
	for(i=0; i<MAX_SHEETS; i++)
	{
	      ctl->sheets0[i].flags = 0;
	      ctl->sheets0[i].ctl = ctl;
	}
	return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
      struct SHEET *sht;
      int i;
      for(i=0; i<MAX_SHEETS; i++)
      {
	if(!ctl->sheets0[i].flags)
	{
	      sht = &ctl->sheets0[i];
	      sht->flags = SHEET_USED;
	      sht->height = -1;
	      sht->task = 0;
	      return sht;
	}
      }
      return 0;
}

void sheet_setbuf(struct SHEET *sht, unsigned char* buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return ;
}

void sheet_updown(struct SHEET* sht, int height)
{
      struct SHTCTL *ctl = sht->ctl; 
      int h, org = sht->height;
      if(height > ctl->top +1)
	      height = ctl->top+1;
      if(height < -1)
	      height = -1;
      sht->height = height;

       if(org > height)
      {
	if(height >= 0)
	{
	      for(h=org; h>height; h--)
	      {
	    	ctl->sheets[h] = ctl->sheets[h-1];
		ctl->sheets[h]->height = h;
  	      }
	      ctl->sheets[height] = sht;
	      sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0+sht->bxsize, 
	      sht->vy0+sht->bysize, height+1);
	      sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0+sht->bxsize, 
	      sht->vy0+sht->bysize, height+1, org);
	}
	else
	{
	       if(ctl->top > org)
	      {
  	            for(h = org; h<ctl->top; h++)
	            {
	      	      ctl->sheets[h] = ctl->sheets[h+1];
		      ctl->sheets[h]->height = h;
		}
		ctl->top--;
	      }
	      sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0+sht->bxsize, sht->vy0+
	      sht->bysize, 0);
	      sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0+sht->bxsize, sht->vy0+
	      sht->bysize, 0, org-1);
	}
      }
      else if(org < height)
      {
	if(org >= 0)
	{
                 for(h=org; h<height; h++)
	     {
	           ctl->sheets[h] = ctl->sheets[h+1];
	           ctl->sheets[h]->height = h;
	     }
	      ctl->sheets[height] = sht;
	}
	else 
	{
	      for(h=ctl->top+1; h>height; h--)
	      {
	 	ctl->sheets[h] = ctl->sheets[h-1];
		ctl->sheets[h]->height = h;
	      }
	      ctl->sheets[height] = sht;
	      ctl->top++;
	}
	sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0+sht->bxsize, sht->vy0+sht->bysize,
	height);
	sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0+sht->bxsize, sht->vy0+sht->bysize,
	height, height);
       }
       return;
}

void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
      struct SHTCTL* ctl = sht->ctl;
      int org_vx0 = sht->vx0;
      int org_vy0 = sht->vy0;
      sht->vx0 = vx0;
      sht->vy0 = vy0;
      if(sht->height >= 0)
      {
	sheet_refreshmap(ctl, org_vx0, org_vy0, org_vx0 + sht->bxsize, org_vy0+sht->bysize, 0);
	sheet_refreshmap(ctl, vx0, vy0, vx0+sht->bxsize, vy0+sht->bysize, sht->height);
	
	sheet_refreshsub(ctl, org_vx0, org_vy0, org_vx0+sht->bxsize, org_vy0+sht->bysize, 0, 
	sht->height-1);
	sheet_refreshsub(ctl, vx0, vy0, vx0+sht->bxsize, vy0+sht->bysize, sht->height, 
	sht->height);
      }
      return;
}

void sheet_free(struct SHEET *sht)
{
      if(sht->height >= 0)
	sheet_updown(sht, -1);
      sht->flags = 0;
      return;
}

void sheet_refresh(struct SHEET *sht, int buf_x0, int buf_y0, int buf_x1, int buf_y1)
{
      if(sht->height >= 0)
      {
	sheet_refreshsub(sht->ctl, sht->vx0+buf_x0, sht->vy0+buf_y0, sht->vx0+buf_x1, 
	sht->vy0+buf_y1, sht->height, sht->height);
      }
      return;
}

void sheet_refreshsub(struct SHTCTL* ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1)
{
      int h, bx, by, vx, vy, bx0, by0, bx1, by1, bx2, sid4, i, i1, *p, *q, *r;
      unsigned char* buf, 	*vram = ctl->vram, *map = ctl->map, sid;
      struct SHEET* sht;
      if(vx0 < 0) vx0 = 0; 
      if(vy0 < 0) vy0 = 0;
      if(vx1 > ctl->xsize) vx1 = ctl->xsize;
      if(vy1 > ctl->ysize) vy1 = ctl->ysize;
      for(h=h0; h<=h1; h++)
      {
	sht = ctl->sheets[h];
	buf = sht->buf;
	sid = sht - ctl->sheets0;	

	bx0 = vx0 - sht->vx0;
	by0 = vy0 - sht->vy0;
	if(bx0 < 0)	bx0 = 0;
	if(by0 < 0)	by0 = 0;
	
	bx1 = vx1 - sht->vx0;
 	by1 = vy1 - sht->vy0;
	
	if(bx1 > sht->bxsize)	bx1 = sht->bxsize;
	if(by1 > sht->bysize)	by1 = sht->bysize;

	if(!(sht->vx0 & 3))
	{
	      i = (bx0 + 3) / 4;
	      i1 = bx1 / 4;
	      i1 = i1 - i;
	      sid4 = sid | sid << 8 | sid << 16 | sid << 24;
	      for(by = by0; by < by1; by++)
	      {
		vy = sht->vy0 + by;
		for(bx = bx0; bx < bx1 && (bx & 3); bx++)
		{
		      vx = sht->vx0 + bx;
		      if(map[vy*ctl->xsize + vx] == sid)
			vram[vy * ctl->xsize + vx] = buf[by*sht->bxsize + bx];
		}
		vx = sht->vx0 + bx;
		p = (int*)&map[vy*ctl->xsize+vx];
		q = (int*)&vram[vy*ctl->xsize+vx];
		r = (int*)&buf[by*sht->bxsize+bx];
		for(i=0; i < i1; i++)
		{
		      if(p[i] == sid4)
			q[i] = r[i];
		      else
		      {
			bx2 = bx + i*4;
			vx = sht->vx0 + bx2;
			if(map[vy*ctl->xsize + vx] == sid)
			      vram[vy*ctl->xsize + vx] = buf[by*sht->bxsize + bx2];
			if(map[vy*ctl->xsize + vx + 1] == sid)
			      vram[vy*ctl->xsize + vx + 1] = buf[by*sht->bxsize + bx2 + 1];
			if(map[vy*ctl->xsize + vx + 2] == sid)
			      vram[vy*ctl->xsize + vx + 2] = buf[by*sht->bxsize + bx2 + 2];
			if(map[vy*ctl->xsize + vx + 3] == sid)
			      vram[vy*ctl->xsize + vx + 3] = buf[by*sht->bxsize + bx2 + 3];
		      }
		}
		for(bx += i1*4; bx < bx1; bx++)
		{
		      vx = sht->vx0 + bx;
		      if(map[vy*ctl->xsize + vx] == sid)
			vram[vy * ctl->xsize + vx] = buf[by*sht->bxsize + bx];
		}
	      }
	}
	else
	{
	      for(by = by0; by < by1; by++)
	      {
	 	vy = sht->vy0 + by;
	 	for(bx = bx0; bx < bx1; bx++)
		{
		      vx = sht->vx0 + bx;
		      if(map[vy*ctl->xsize + vx] == sid)
			vram[vy * ctl->xsize + vx] = buf[by*sht->bxsize + bx];
		}
	      }
	}
      }
      return ;
}

void sheet_refreshmap(struct SHTCTL* ctl, int vx0, int vy0, int vx1, int vy1, int h0)
{
      int h, bx, by, vx, vy, bx0, by0, bx1, by1, sid4, *p;
      unsigned char* buf, sid, *map = ctl->map;
      struct SHEET *sht;
      if(vx0 < 0)	vx0 = 0;
      if(vy0 < 0) 	vy0 = 0;
      if(vx1 > ctl->xsize)	vx1 = ctl->xsize;
      if(vy1 > ctl->ysize)	vy1 = ctl->ysize;
      
      for(h = h0; h<=ctl->top; h++)
      {
	sht = ctl->sheets[h];
	sid = sht - ctl->sheets0;
	buf = sht->buf;
	bx0 = vx0 - sht->vx0;
	by0 = vy0 - sht->vy0;
	if(bx0 < 0)	bx0 = 0;
	if(by0 < 0)	by0 = 0;
	
	bx1 = vx1 - sht->vx0;
	by1 = vy1 - sht->vy0;
	if(bx1 > sht->bxsize)	bx1 = sht->bxsize;
	if(by1 > sht->bysize)	by1 = sht->bysize;

	if(sht->col_inv == -1)
	{
	      if(!(sht->vx0 & 3) && !(bx0 & 3) && !(bx1 & 3))
	      {
		bx1 = ( bx1 - bx0 ) / 4;
		sid4 = sid | sid << 8 | sid << 16 | sid << 24;
		for(by = by0; by < by1; by++)
		{
		      vy = sht->vy0+by;    
		      vx = sht->vx0 + bx0;
		      p = (int*)&map[vy*ctl->xsize + vx];
		      for(bx = 0; bx < bx1; bx++)
			p[bx] = sid4;
		}
	      }
	      else
	      {
		for(by = by0; by < by1; by++)
		{
		      vy = by+sht->vy0;
		      for(bx = bx0; bx < bx1; bx++)
		      {
			vx = bx + sht->vx0;
			map[vy*ctl->xsize + vx] = sid;
		      }
		}
	      }
	}
	else
	{
	      for(by = by0; by < by1; by++)
	      {
		vy = by+sht->vy0;
		for(bx = bx0; bx < bx1; bx++)
		{
		      vx = bx + sht->vx0;
		      if(buf[by*sht->bxsize + bx] != sht->col_inv)
			map[vy*ctl->xsize + vx] = sid;
		}
	      }
	}	
      }
}

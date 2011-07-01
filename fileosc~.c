/* Marcelo Queiroz, junho/2011 */

#include "m_pd.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------ fileosc~ ----------------------------- */

static t_class *fileosc_class;

typedef struct _fileosc
{
  t_object x_obj;       /* obligatory header */
  t_float x_f;          /* place to hold inlet's value if it's set by message */
  t_int S; /* tamanho da tabela SENO */
  t_float *SENO; /* a tabela */
  FILE *arq; /* o arquivo de entrada */
  t_float freq; /* valor de frequencia */
  t_float ind; /* indice de leitura do oscilador */
  t_int dur; /* duracao do evento atual */
  t_float delta; /* variacao do indice */
  t_int n; /* indice da amostra atual */
  t_float stat; /* diz se e' pra produzir audio ou nao */
  t_outlet *bangoutlet; /* saida dizendo que acabou o processamento */
} t_fileosc;


static void fileosc_bang(t_fileosc *x) {
  int i,y,z;
  if (x->arq) {
    x->stat = 1;
    x->n=0;
    rewind(x->arq);
    x->ind=0;
    post("comecando a ler o arquivo...");
  }
  else
    post("abra o arquivo primeiro!");
}

static void fileosc_anything(t_fileosc *x, t_symbol *s, int argc, t_atom *argv)
{
  if (!strcmp(s->s_name,"open"))
    {
      t_symbol *n = atom_getsymbol(&argv[0]);
      x->arq = fopen(n->s_name,"r");
      if (!x->arq)
        post("nao consegui abrir o arquivo");
      else post("abri o arquivo");
    }
}

/* this is the actual performance routine which acts on the samples.
   It's called with a single pointer "w" which is our location in the
   DSP call list.  We return a new "w" which will point to the next item
   after us.  Meanwhile, w[0] is just a pointer to dsp-perform itself
   (no use to us), w[1] and w[2] are the input and output vector locations,
   and w[3] is the number of points to calculate. */
static t_int *fileosc_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  int n = (int)(w[3]);
  t_fileosc *x = (t_fileosc *)(w[4]);

  double y,ind;
  int z,v;

  if (x->stat)
    {
      while (n--)
        {
	  /* trata o inicio de eventos */
          if (x->n==0)
            { 
	      /* testa se acabou arquivo */
              z=fscanf(x->arq,"%lf",&y);
              x->freq=y;
              if (feof(x->arq))
                {
                  x->stat=0;
                  x->n = 0;
                  post("...acabou o arquivo!");
                  outlet_bang(x->bangoutlet);
                  return (w+5);
                }
	      z=fscanf(x->arq,"%d",&v);
              x->dur=v;
	      post("novo evento: frequencia=%lf, duracao=%d",x->freq,x->dur);
	      x->delta = x->freq*x->S/sys_getsr();
            }
	  /* Aqui esta' sendo feita a leitura truncada da tabela */
	  ind = x->ind+x->n*x->delta;
	  *out = x->SENO[(int)(ind-((int)ind/x->S)*x->S)];
          x->n++;
          if (x->n==x->dur)
	    {
              x->n=0;
	      x->ind = x->ind+x->n*x->delta;
	    }
          *out++;
        }
    }
  return (w+5);
}

/* called to start DSP.  Here we call Pd back to add our perform
   routine to a linear callback list which Pd in turn calls to grind
   out the samples. */
static void fileosc_dsp(t_fileosc *x, t_signal **sp)
{
  dsp_add(fileosc_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n, x);
}

static void *fileosc_new(void)
{
  int i;
  t_fileosc *x = (t_fileosc *)pd_new(fileosc_class);
  outlet_new(&x->x_obj, gensym("signal"));
  x->bangoutlet = outlet_new(&x->x_obj, gensym("bang"));

  post("inicializando objeto fileosc~...\ncomputando tabela do SENO...");
  x->S = 1024;
  x->SENO = (t_float*)malloc(x->S*sizeof(t_float));
  for (i=0;i<x->S;i++)
    x->SENO[i] = sin(2*M_PI*i/x->S);
  x->stat = 0;
  return (x);
}

/* this routine, which must have exactly this name (with the "~" replaced
   by "_tilde) is called when the code is first loaded, and tells Pd how
   to build the "class". */
void fileosc_tilde_setup(void)
{
  post("inicializando classe fileosc~...");
  fileosc_class = class_new(gensym("fileosc~"), (t_newmethod)fileosc_new, 0,
                                  sizeof(t_fileosc), 0, A_DEFFLOAT, 0);
  /* this is magic to declare that the leftmost, "main" inlet
     takes signals; other signal inlets are done differently...*/
  CLASS_MAINSIGNALIN(fileosc_class, t_fileosc, x_f);
  /* here we tell Pd about the "dsp" method, which is called back
     when DSP is turned on. */
  class_addmethod(fileosc_class, (t_method)fileosc_dsp, gensym("dsp"), 0);
  class_addbang(fileosc_class, (t_method)fileosc_bang);
  class_addanything(fileosc_class, fileosc_anything);
}

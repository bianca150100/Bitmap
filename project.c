#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
typedef struct
{
    int st_jos[2];
    int dr_sus[2];
} fereastra;

typedef struct
{
    unsigned int tip;
    float corelatie;
    fereastra contur;
} detectie;

typedef struct
{
    unsigned int R;
    unsigned int G;
    unsigned int B;
} culoare;

unsigned int * XORSHIFT32(unsigned int seed, unsigned int n)
{
    unsigned int k, *v, r;
    v=(unsigned int*)malloc(n*sizeof(unsigned int));
    r=seed;
    for(k=0; k<n; k++)
    {
        r= r ^ r <<13;
        r= r ^ r >>17;
        r= r ^ r <<5;
        v[k]=r;
    }
    return v;
}
unsigned char* liniarizare_imagine(char *nume_fisier_sursa, unsigned int *dim, unsigned int *inaltime_img, unsigned int *latime_img,unsigned char **header)
{
    FILE *f;
    printf("Numele fisierului sursa: %s \n", nume_fisier_sursa);
    f= fopen(nume_fisier_sursa, "rb");
    if (f==NULL)
    {
        printf("Eroare la deschiderea imaginii");
        return 0;
    }
    *header= (unsigned char*)malloc(54*sizeof(unsigned char));
    //citim headerul
    fread(*header,sizeof(unsigned char),54,f);
    unsigned int d,w,h;
    fseek ( f ,2, SEEK_SET );
    //citim dimensiunea, inaltimea si latimea
    fread(&d, sizeof(unsigned int),1, f);
    fseek(f, 18, SEEK_SET);
    fread(&w, sizeof(unsigned int), 1, f);
    fread(&h, sizeof(unsigned int), 1, f);
    fseek(f,0,SEEK_SET);
    fseek(f,54,SEEK_SET);
    int i;
    unsigned char *v;
    v=(unsigned char*)malloc(3*w*h*sizeof(unsigned char));
    //in v retinem pixelii
    for (i=0; i <3*w*h; i=i+3)
    {
        unsigned char pRGB[3];
        fread(&pRGB, 3, 1, f);
        v[i]=pRGB[2];
        v[i+1]=pRGB[1];
        v[i+2]=pRGB[0];

    }
    *inaltime_img=h;
    *latime_img=w;
    *dim=d;
    fclose(f);
    return v;

}


void creare_img(char *nume,char *nume_fisier_destinatie,unsigned char *p,unsigned int dim, unsigned int latime, unsigned int inaltime, unsigned char *header)
{

    FILE *f=fopen(nume_fisier_destinatie,"wb");
    FILE *g=fopen(nume,"rb");
    int padding;
    //calculam padding
    if(latime % 4 != 0)
        padding = (4 - (3 * latime) % 4)%4;
    else
        padding = 0;
    int i,j;
    unsigned int paddingnull=0;
    int k;
    //scriem headerul in noua imagine
    fwrite(header,sizeof(unsigned char),54,f);
    int q=0;
    //scriem fiecare pixel, adaugand paddingul
    for(i=0; i<inaltime; i++)
    {
        for(j=0; j<latime; j++)
        {
            fwrite((p+q+2),sizeof(unsigned char),1,f);
            fwrite((p+q+1),sizeof(unsigned char),1,f);
            fwrite((p+q+0),sizeof(unsigned char),1,f);
            fflush(f);
            q=q+3;
        }
        for(k=0; k<padding; k++)
            fwrite(&paddingnull,sizeof(unsigned char),3,f);
    }
    fclose(f);
    fclose(g);
}

int test_chi_patrat(char *nume)
{
    unsigned char *p,*header;
    unsigned int dim,inaltime,latime;
    p=liniarizare_imagine(nume,&dim,&inaltime,&latime,&header);
    unsigned int *r,*g,*b;
    r=(unsigned int*)calloc(256,sizeof(unsigned int));
    b=(unsigned int*)calloc(256,sizeof(unsigned int));
    g=(unsigned int*)calloc(256,sizeof(unsigned int));
    double f,sr=0,sg=0,sb=0;
    f=(inaltime*latime)/256;
    int i;
    for(i=0; i<3*inaltime*latime; i++)
    {
        if(i%3==0)
            b[p[i]]++;
        if(i%3==1)
            g[p[i]]++;
        if(i%3==2)
            r[p[i]]++;
    }
    for(i=0; i<256; i++)
    {
        sb=(b[i]-f)*(b[i]-f)/f+sb;
        sg=(g[i]-f)*(g[i]-f)/f+sg;
        sr=(r[i]-f)*(r[i]-f)/f+sr;
    }
    printf ("%lf \n %lf \n %lf\n",sb,sg,sr);
    free(r);
    free(g);
    free(b);
}

unsigned char* criptare(char *nume_img_initiala, char *nume_img_criptata, char *cheia_secreta)
{
    FILE *f=fopen(nume_img_initiala,"rb");
    FILE *g=fopen(nume_img_criptata,"wb");
    FILE *h=fopen(cheia_secreta,"r");
    if(f==NULL || g==NULL || h==NULL)
    {
        printf("eroare la deschiderea unui fisier");
        return 0;
    }
    unsigned int R0,SV;
    int i;
    fscanf(h,"%u",&R0);
    fscanf(h,"%u",&SV);
    unsigned int dim,latime,inaltime,*v,a,r;
    unsigned char *p,*header;
    p=liniarizare_imagine(nume_img_initiala,&dim,&inaltime,&latime,&header);
    //generam secventa de 2*latime*ianltime-1 de numere aleatoare
    v=XORSHIFT32(R0,2*inaltime*latime-1);
    unsigned int *permutare;
    permutare=(unsigned int*)malloc((inaltime*latime)*sizeof(unsigned int));
    for(i=0; i<=inaltime*latime-1; i++)
        permutare[i]=i;
    //generam permutarea aleatoare
    for(i=inaltime*latime-1; i>=0; i--)
    {
        unsigned int aux;
        a=v[inaltime*latime-1-i];
        r=a%(i+1);
        aux=permutare[i];
        permutare[i]=permutare[r];
        permutare[r]=aux;
    }
    unsigned char *auxiliar;
    auxiliar=(unsigned char*)malloc(3*inaltime*latime*sizeof(unsigned char));
    for(i=0; i<inaltime*latime*3; i++)
        auxiliar[i]=0;
    //permutam pixelii conform permutarii obtinute
    for(i=0; i<=inaltime*latime-1; i++)
    {
        unsigned char x,y,z;
        unsigned int o,u;
        o=3*i;
        u=3*(permutare[i]);
        x=p[o];
        auxiliar[u]=x;
        y=p[o+1];
        auxiliar[u+1]=y;
        z=p[o+2];
        auxiliar[u+2]=z;

    }
    //aplicam procesul de XOR
    unsigned char x,y,z;
    z=SV & 255;
    y= (SV>>8) & 255 ;
    x= (SV>>16) & 255;
    auxiliar[0]=x ^ auxiliar[0];
    auxiliar[1]=y ^ auxiliar[1];
    auxiliar[2]=z ^ auxiliar[2];
    z= v[inaltime*latime] & 255;
    y= (v[inaltime*latime]>>8) & 255;
    x= (v[inaltime*latime]>>16) & 255;
    auxiliar[0]=x ^ auxiliar[0];
    auxiliar[1]=y ^ auxiliar[1];
    auxiliar[2]=z ^ auxiliar[2];
    for(i=1; i<=inaltime*latime-1; i++)
    {
        auxiliar[3*i]=auxiliar[3*(i-1)] ^ auxiliar[3*i];
        auxiliar[3*i +1]=auxiliar[3*(i-1)+1] ^ auxiliar[3*i+1];
        auxiliar[3*i+2]=auxiliar[3*(i-1)+2] ^ auxiliar[3*i+2];
        z= v[inaltime*latime+i] & 255;
        y= (v[inaltime*latime+i]>>8) & 255;
        x= (v[inaltime*latime+i]>>16) & 255;
        auxiliar[3*i]=x ^ auxiliar[3*i];
        auxiliar[3*i +1]=y ^ auxiliar[3*i+1];
        auxiliar[3*i +2]=z ^ auxiliar[3*i+2];
    }
    fclose(f);
    fclose(g);
    fclose(h);
    creare_img(nume_img_initiala,nume_img_criptata,auxiliar,dim,latime,inaltime,header);
    free(permutare);
    free(p);
    free(header);
    return auxiliar;


}

unsigned char* decriptare(char *nume_img_initiala, char *nume_img_decriptata, char *cheia_secreta)
{
    FILE *f=fopen(nume_img_initiala,"rb");
    FILE *g=fopen(nume_img_decriptata,"wb");
    FILE *h=fopen(cheia_secreta,"r");
    if(f==NULL || g==NULL || h==NULL)
    {
        printf("eroare la deschiderea unui fisier");
        return 0;
    }
    unsigned int R0,SV;
    int i;
    fscanf(h,"%u",&R0);
    fscanf(h,"%u",&SV);
    unsigned int dim,latime,inaltime,*v,a,r;
    unsigned char *p,*header,*ff;
    p=liniarizare_imagine(nume_img_initiala,&dim,&inaltime,&latime,&header);
    //in ff retinem o copie a vectorului liniarizat, ajutandu-ne ulterior la XORare
    ff=liniarizare_imagine(nume_img_initiala,&dim,&inaltime,&latime,&header);
    unsigned char *auxiliar;
    auxiliar=(unsigned char*)malloc(3*inaltime*latime*sizeof(unsigned char));
    for(i=0; i<inaltime*latime*3; i++)
        auxiliar[i]=0;
    v=XORSHIFT32(R0,2*inaltime*latime-1);
    unsigned char x,y,z;
    unsigned int *permutare,*permutare_inv;
    permutare=(unsigned int*)malloc((inaltime*latime)*sizeof(unsigned int));
    permutare_inv=(unsigned int*)malloc((inaltime*latime)*sizeof(unsigned int));
    //generam permutarea
    for(i=0; i<=inaltime*latime-1; i++)
        permutare[i]=i;
    for(i=inaltime*latime-1; i>=0; i--)
    {
        unsigned int aux;
        a=v[inaltime*latime-1-i];
        r=a%(i+1);
        aux=permutare[i];
        permutare[i]=permutare[r];
        permutare[r]=aux;
    }
    //calculam permutarea
    for(i=0; i<=inaltime*latime-1; i++)
    {
        permutare_inv[permutare[i]]=i;
    }
    //aplicam XOR
    z=SV & 255;
    y= (SV>>8) & 255 ;
    x= (SV>>16) & 255;
    p[0]=x ^ p[0];
    p[1]=y ^ p[1];
    p[2]=z ^ p[2];
    z= v[inaltime*latime] & 255;
    y= (v[inaltime*latime]>>8) & 255;
    x= (v[inaltime*latime]>>16) & 255;
    p[0]= p[0] ^ x;
    p[1]= p[1] ^ y ;
    p[2]= p[2] ^ z;
    for(i=1; i<=inaltime*latime-1; i++)
    {
        p[3*i]=ff[3*(i-1)] ^ ff[3*i];
        p[3*i +1]=ff[3*(i-1)+1] ^ ff[3*i+1];
        p[3*i+2]=ff[3*(i-1)+2] ^ ff[3*i+2];
        z= v[inaltime*latime+i] & 255;
        y= (v[inaltime*latime+i]>>8) & 255;
        x= (v[inaltime*latime+i]>>16) & 255;
        p[3*i]=x ^ p[3*i];
        p[3*i +1]=y ^ p[3*i+1];
        p[3*i+2]=z ^ p[3*i+2];
    }
    //permutam pixelii
    for(i=0; i<=inaltime*latime-1; i++)
    {
        unsigned char x,y,z;
        unsigned int o,u;
        o=3*i;
        u=3*(permutare_inv[i]);
        x=p[o];
        auxiliar[u]=x;
        y=p[o+1];
        auxiliar[u+1]=y;
        z=p[o+2];
        auxiliar[u+2]=z;

    }
    fclose(f);
    fclose(g);
    fclose(h);
    creare_img(nume_img_initiala,nume_img_decriptata,auxiliar,dim,latime,inaltime,header);
    free(permutare);
    free(permutare_inv);
    free(ff);
    free(header);
    free(p);
    return auxiliar;
}

detectie *match(unsigned int inaltime_img, unsigned int latime_img, unsigned char *p, unsigned int inaltime_template, unsigned int latime_template, unsigned char *v, unsigned int tip, int *D_lung, float ps)
{
    unsigned int  i,j,q,aux,padding;
    q=0;
    //calculez paddingul pentru imagine
    if(latime_img % 4 != 0)
        padding = (4 - (3 * latime_img) % 4)%4;
    else
        padding = 0;
    //transform imaginea in tonuri de gri
    for(i=0; i<inaltime_img; i++)
    {
        for(j=0; j<latime_img; j++)
        {
            aux=0.299*p[q]+0.587*p[q+1]+0.114*p[q+2];
            p[q]=aux;
            p[q+1]=aux;
            p[q+2]=aux;
            q=q+3;
        }

        q=q+padding*3;
    }

    //calculez padding pt template
    q=0;
    if(latime_template % 4 != 0)
        padding = (4 - (3 * latime_template) % 4)%4;
    else
        padding = 0;

    // transform template-ul in tonuri de gri
    for(i=0; i<inaltime_template; i++)
    {
        for(j=0; j<latime_template; j++)
        {
            aux=0.299*v[q]+0.587*v[q+1]+0.114*v[q+2];
            v[q]=aux;
            v[q+1]=aux;
            v[q+2]=aux;
            q=q+3;
        }

        q=q+padding*3;
    }

    //completam imaginea cu 0 astfel incat sa putem parcurge  x-y pe toate coordonatele img
    //sa putem pune coltul st_jos template in coltul dr_sus imagine si dr_sus template in st_jos imagine
    unsigned int inaltime;
    inaltime=inaltime_img+2*inaltime_template-2;
    unsigned int latime;
    latime=latime_img+2*latime_template-2;
    unsigned char *im_new;
    im_new=(unsigned char*)malloc(3*latime*inaltime*sizeof(unsigned char));
    q=0;
    for(i=0; i<inaltime; i++)
    {
        if(i<inaltime_template-1 || i>inaltime-inaltime_template)
        {
            for(j=0; j<latime; j++)
            {
                im_new[q]=0;
                im_new[q+1]=0;
                im_new[q+2]=0;
                q=q+3;
            }
        }
        else
        {
            for(j=0; j<latime_template-1; j++)
            {
                im_new[q]=0;
                im_new[q+1]=0;
                im_new[q+2]=0;
                q=q+3;
            }
            for(; j<latime-latime_template+1; j++)
            {
                im_new[q]=p[3*latime_img*(i-inaltime_template+1)+3*(j-latime_template+1)];
                im_new[q+1]=p[3*latime_img*(i-inaltime_template+1)+3*(j-latime_template+1)];
                im_new[q+2]=p[3*latime_img*(i-inaltime_template+1)+3*(j-latime_template+1)];
                q=q+3;
            }
            for(; j<latime; j++)
            {
                im_new[q]=0;
                im_new[q+1]=0;
                im_new[q+2]=0;
                q=q+3;
            }
        }
    }

    //calculam corelatia pt fiecare punct
    float Tmed, Imed, Tdev, Idev, corr;
    int k,l;
    float x,y,dif;
    //aflam medie template
    Tmed=0;
    for(i=0; i<inaltime_template; i++)
        for(j=0; j<latime_template; j++)
        {
            Tmed=Tmed+v[3*(latime_template*i+j)];
        }
    Tmed=Tmed/(inaltime_template*latime_template);

    //aflam deviatia template
    Tdev=0;
    for(i=0; i<inaltime_template; i++)
        for(j=0; j<latime_template; j++)
        {
            dif=v[3*(latime_template*i+j)]-Tmed;
            Tdev=Tdev+dif*dif;
        }
    Tdev=Tdev/(inaltime_template*latime_template-1);
    Tdev=sqrt(Tdev);
    typedef struct
    {
        unsigned int a,b;
        float c;
    } pereche;
    pereche *per;
    per=(pereche*)malloc(sizeof(pereche)*(3*inaltime*latime/(inaltime_template*latime_template)));
    int nr_per=0;
    float *cor;
    cor=(float*)malloc(sizeof(float)*(3*inaltime*latime/(inaltime_template*latime_template)));
    for(i=0; i<inaltime-inaltime_template+1; i++)
    {
        for(j=0; j<latime-latime_template+1; j++)
        {
            //aflare medii
            Imed=0;
            for(k=0; k<inaltime_template; k++)
            {
                for(l=0; l<latime_template; l++)
                {
                    Imed=Imed+im_new[3*(latime*(i+k)+(j+l))];
                }
            }
            Imed=Imed/(inaltime_template*latime_template);
            //aflare deviatii
            Idev=0;
            for(k=0; k<inaltime_template; k++)
            {
                for(l=0; l<latime_template; l++)
                {
                    dif=im_new[3*(latime*(i+k)+(j+l))]-Imed;
                    Idev=Idev+dif*dif;
                }
            }
            Idev=Idev/(inaltime_template*latime_template-1);
            Idev=sqrt(Idev);
            //calcularea corelatiei
            corr=0;
            for(k=0; k<inaltime_template; k++)
            {
                for(l=0; l<latime_template; l++)
                {
                    x=im_new[3*(latime*(i+k)+(j+l))]-Imed;
                    y=v[3*(latime_template*k+l)]-Tmed;
                    corr=corr+(x*y)/(Idev*Tdev);
                }
            }
            corr=corr/(inaltime_template*latime_template);
            if(corr>=ps)
            {
                per[nr_per].a=i;
                per[nr_per].b=j;
                cor[nr_per]=corr;
                nr_per++;
            }
        }
    }
    detectie *D;
    D=(detectie*)malloc(nr_per*sizeof(detectie));
    fereastra f;
    for (i=0; i<nr_per; i++)
    {
        f.st_jos[0]=per[i].a-inaltime_template+1;
        f.st_jos[1]=per[i].b-inaltime_template+1;
        f.dr_sus[0]=per[i].a;
        f.dr_sus[1]=per[i].b;
        D[i].contur=f;
        D[i].corelatie=cor[i];
        D[i].tip=tip;
    }
    *D_lung=nr_per;
    free(im_new);
    free(cor);
    free(per);
    return D;

}

int min(int a,int b)
{
    if(a>b)
        return b;
    return a;
}

int max(int a, int b)
{
    if(a>b)
        return a;
    return b;
}

unsigned char* contur( unsigned char *p,unsigned int dim, unsigned int inaltime_img, unsigned int latime_img,  fereastra f, culoare C )
{
    int rand,coloana,pozitie;
    //trasam dreptunghiul parcurgand fiecare linie si coloana corespunzatoare si colorand utilizand C
    for(rand=max(0,f.st_jos[0]); rand<=min(inaltime_img,f.dr_sus[0]); rand++)
    {
        coloana=f.st_jos[1];
        if(coloana>0 && coloana <latime_img)
        {
            pozitie=3*(rand*latime_img+coloana);
            p[pozitie]=C.R;
            p[pozitie+1]=C.G;
            p[pozitie+2]=C.B;
        }
        coloana=f.dr_sus[1];
        if(coloana>0 && coloana<latime_img)
        {
            pozitie=3*(rand*latime_img+coloana);
            p[pozitie]=C.R;
            p[pozitie+1]=C.G;
            p[pozitie+2]=C.B;
        }
    }
    for(coloana=max(0,f.st_jos[1]); coloana<=min(latime_img,f.dr_sus[1]); coloana++)
    {
        rand=f.st_jos[0];
        if(rand>0 && rand<inaltime_img)
        {
            pozitie=3*(rand*latime_img+coloana);
            p[pozitie]=C.R;
            p[pozitie+1]=C.G;
            p[pozitie+2]=C.B;
        }
        rand=f.dr_sus[0];
        if(rand>0 && rand<inaltime_img)
        {
            pozitie=3*(rand*latime_img+coloana);
            p[pozitie]=C.R;
            p[pozitie+1]=C.G;
            p[pozitie+2]=C.B;
        }
    }
    return p;

}



int comp_int(const void*a, const void *b) //functia ne va ajuta ulterior sa comparam indicii la suprapunere
{
    int x;
    int y;
    x=*(int*)a;
    y=*(int*)b;
    return x-y;
}

int sunt_suprapuse( detectie D1, detectie D2)
{
    int arie_intersectie;
    int arie1, arie2;
    //calculam aria celor doua detectii
    arie1=(D1.contur.dr_sus[0]-D1.contur.st_jos[0]+1)*(D1.contur.dr_sus[1]-D1.contur.st_jos[1]+1);
    arie2=(D2.contur.dr_sus[0]-D2.contur.st_jos[0]+1)*(D2.contur.dr_sus[1]-D2.contur.st_jos[1]+1);
    if (D1.contur.st_jos[0]>D2.contur.dr_sus[0] || D1.contur.dr_sus[0]< D2.contur.st_jos[0])
        return 0;  //nu se suprapun
    if (D1.contur.st_jos[1]>D2.contur.dr_sus[1] || D1.contur.dr_sus[1] <D2.contur.st_jos[1])
        return 0; //nu se suprapun
    //daca s-a ajuns aici, stim ca se intersecteaza.
    //pentru a afla intersectia, sortam coordonatele fiecarei detectii, iat coordonatele din mojloc vor reprezenta colturile intersectiei
    int v[4],p[4];
    v[0]=D1.contur.st_jos[0];
    v[1]=D1.contur.dr_sus[0];
    v[2]=D2.contur.st_jos[0];
    v[3]=D2.contur.dr_sus[0];
    p[0]=D1.contur.st_jos[1];
    p[1]=D1.contur.dr_sus[1];
    p[2]=D2.contur.st_jos[1];
    p[3]=D2.contur.dr_sus[1];
    qsort(v,4,sizeof(int),comp_int);
    qsort(p,4,sizeof(int),comp_int);
    //calculam aria intersectiei
    arie_intersectie=(v[2]-v[1]+1)*(p[2]-p[1]+1);
    if ((float)arie_intersectie/(arie1+arie2-arie_intersectie)>0.2)
        return 1;
    return 0;
}

int cmp(const void*a, const void*b) //functia va ajuta sa sortam descrescator vectorul de detectii
{
    if(((detectie*)a)->corelatie > ((detectie*)b)->corelatie)
        return -1;
    return 1;
}

detectie* sortare_detectii(detectie *d, int n)
{
    qsort(d,n,sizeof(detectie),cmp);
    return d;
}

detectie* elimina_nonmaxime(detectie *D, int *lungime)
{
    unsigned int i,j,elim=0;
    int n;
    n=*lungime;
    //se sorteaza descrescator dupa corelatie
    D= sortare_detectii(D,n);
    //eliminam detectiile care se suprapun, facandu-le corelatia 0
    for(i=0; i<n-1; i++)
        if(D[i].corelatie)
            for(j=i+1; j<n; j++)
                if(D[j].corelatie !=0 && sunt_suprapuse(D[i],D[j])!=0)
                {
                    D[j].corelatie=0;
                    elim++;
                }
    detectie *aux;
    aux=(detectie*)malloc((n-elim)*sizeof(detectie));
    j=0;
    for(i=0; i<n; i++)
        if(D[i].corelatie!=0)
        {
            aux[j]=D[i];
            j++;
        }
    free (D);
    *lungime=j;
    return aux;
}


int main()
{
    unsigned char *p,*header,*v,*fin,*theader,*templ;
    unsigned int a,b,c;

    detectie  *aux[100],*D;
    unsigned int D_AUX, nr_sablon=0, nr_detect[100], nr_detect_total=0, tdim, th, tw;
    char w[1], s[10][45]= {"",""};
    culoare cul[]= {{255, 0, 0} , {255, 255, 0} , {0, 255, 0} , {0, 255, 255}, {255, 0, 255} , {0, 0, 255} , {192, 192, 192} , {255, 140, 0} , {128, 0, 128} , {128, 0 ,0}};
    int i;
    char nume[100],nume2[1000];

    //criptare si decriptare//
    printf("Dati numele imaginii ce urmeaza a fi criptata: \n");
    scanf("%s",nume);
    printf("Dati numele fiserului in care se afla cheia secreta: \n");
    scanf("%s",nume2);
    p=liniarizare_imagine(nume,&a,&b,&c,&header);
    p=criptare(nume,"criptare.bmp",nume2);
    creare_img(nume,"criptare.bmp",p,a,c,b,header);
    p=decriptare("criptare.bmp","decriptare.bmp",nume2);
    creare_img(nume,"decriptare.bmp",p,a,c,b,header);
    printf("Dati numele imaginii pt care calculati chi_patrat: %s, \n");
    scanf("%s",nume);
    test_chi_patrat(nume);

    //template maching

    p=liniarizare_imagine(nume,&a,&b,&c,&header);
    for(i=0; i<=9; i++)
    {
        s[i][45]="";
        strcpy(s[i],"Cifra");
        itoa(i,w,10);
        s[i][strlen(s[i])]=w[0];
        w[0]=NULL;
        s[i][strlen(s[i])]=NULL;
    }
    //calculam detectia pt fiecare fereastra
    for(i=0; i<=9; i++)
    {
        strcat(s[i],".bmp");
        templ=liniarizare_imagine(s[i],&tdim,&th,&tw,&theader);
        D_AUX=0;
        aux[i]=match(b,c,p,th,tw,templ,i,&D_AUX,0.5);
        nr_detect[i]=D_AUX;
    }
    nr_detect_total=0;
    //calculam numarul total al detectiilor
    for(i=0; i<=9; i++)
        nr_detect_total=nr_detect_total+nr_detect[i];
    D=(detectie*)malloc(nr_detect_total*sizeof(detectie));
    int j,k;
    k=0;
    //in D retin toate detectiile ale fiecarei cifre
    for(i=0; i<=9; i++)
        for(j=0; j<nr_detect[i]; j++)
        {
            D[k]=aux[i][j];
            k++;
        }
    //sortam detectiile, eliminam nonmaximele si ulterior trasam conturul in fuctie de cifra(culoare)
    D=sortare_detectii(D,nr_detect_total);
    D=elimina_nonmaxime(D,&nr_detect_total);
    for(i=0; i<nr_detect_total; i++)
        fin=contur(p,a,b,c,D[i].contur,cul[D[i].tip]);
    creare_img("test.bmp","final.bmp",fin,a,c,b,header);
    free(D);
    free(templ);
    free(p);
    free(header);
    free(theader);



}

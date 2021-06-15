/*
 *
 * rewrite.c
 * Friday, 5/5/1995.
 *
 */

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GnuFile.h>
#include <GnuStr.h>
#include <GnuMem.h>
#include <GnuMisc.h>


typedef struct _tree
   {
   PSZ *ppsz;
   struct _tree *left;
   struct _tree *right;
   } TREE;
typedef TREE *PTREE;


void MakeDate (PSZ pszDate, PSZ pszSrc)
   {
   *pszDate++ = pszSrc[2];
   *pszDate++ = pszSrc[3];
   *pszDate++ = '/';
   *pszDate++ = pszSrc[4];
   *pszDate++ = pszSrc[5];
   *pszDate++ = '/';
   *pszDate++ = pszSrc[0];
   *pszDate++ = pszSrc[1];
   *pszDate   = '\0';
   }


void WriteTree (PTREE pt, FILE *fpOut)
   {
   PSZ  *ppsz;
   char szDate[16];

   if (!pt)
      return;
   WriteTree (pt->left, fpOut);
   ppsz = pt->ppsz;
   MakeDate (szDate, ppsz[5]);
   fprintf (fpOut, "%-12s %7.7s %s %s\n", ppsz[1], ppsz[3], szDate, ppsz[6]);
   WriteTree (pt->right, fpOut);
   }


PTREE FreeTree  (PTREE pt)
   {
   if (!pt)
      return NULL;
   FreeTree (pt->left);
   FreeTree (pt->right);
   MemFreePPSZ (pt->ppsz, 0);
   free (pt);
   return NULL;
   }


PTREE AddToTree (PTREE *ppt, PSZ *ppsz)
   {
   if (!*ppt)
      {
      *ppt = malloc (sizeof (TREE));
      (*ppt)->left = (*ppt)->right = NULL;
      (*ppt)->ppsz = ppsz;
      return *ppt;
      }
   if (stricmp ((*ppt)->ppsz[5], ppsz[5]) < 0)
      AddToTree (&(*ppt)->left, ppsz);
   else
      AddToTree (&(*ppt)->right, ppsz);
   return *ppt;
   }


USHORT TreeSize (PTREE pt)
   {
   if (!pt)
      return 0;
   return 1 + TreeSize (pt->left) + TreeSize (pt->right);
   }


PTREE DoTheTree (PTREE pt, FILE *fpOut)
   {
   if (!pt)
      return NULL;
   fprintf (fpOut, "\n***** [%s %3.3d files] ***************************************\n", pt->ppsz[0]+6, TreeSize (pt));
   WriteTree (pt, fpOut);       // write stored tree
   printf (".");
   return FreeTree (pt);
   }


void ByDate (FILE *fp, FILE *fpOut)
   {
   char     szLine [512], szCurrDir[128];
   PSZ      psz, *ppsz;
   USHORT   uCols;
   PTREE    pt = NULL;

   *szCurrDir = '\0';
   while (FilReadLine (fp, szLine, NULL, sizeof (szLine)) != 0xFFFF)
      {
      psz  = szLine+10; //skip  "SimTel/",
      ppsz = StrMakePPSZ (psz, ",", TRUE, TRUE, &uCols);
      if (stricmp (ppsz[0], szCurrDir))
         {
         strcpy (szCurrDir, ppsz[0]);
         pt = DoTheTree (pt, fpOut);
         }
      AddToTree (&pt, ppsz);
      }
   pt = DoTheTree (pt, fpOut);
   }



void ByName (FILE *fp, FILE *fpOut)
   {
   char     szLine [512], szCurrDir[128], szDate[16];
   PSZ      psz, *ppsz;
   USHORT   uCols;

   while (FilReadLine (fp, szLine, "", sizeof (szLine)) != 0xFFFF)
      {
      psz = szLine+10; //skip  "SimTel/",

      // 0  Subdirectory   15   // 4  Type#           1
      // 1  Name           12   // 5  Date#           6
      // 2  Vr#             1   // 6  Description    46
      // 3  Size#           7
      ppsz = StrMakePPSZ (psz, ",", TRUE, TRUE, &uCols);

      if (stricmp (ppsz[0], szCurrDir))
         {
         strcpy (szCurrDir, ppsz[0]);
         fprintf (fpOut, "\n***** [%s] ***************************************\n", szCurrDir+5);
         printf (".");
         }
      MakeDate (szDate, ppsz[5]);
      fprintf (fpOut, "%-12s %7.7s %s %s\n", ppsz[1], ppsz[3], szDate, ppsz[6]);
      MemFreePPSZ (ppsz, uCols);
      }
   }


int main (int argc, char *argv[])
   {
   FILE *fp, *fpOut;

   if (!(fp = fopen ("simibm.idx", "rt")))
      Error ("cannot open simibm.idx");

   if (!(fpOut = fopen ("sim-name.txt", "wt")))
      Error ("cannot open sim-name.txt");
   printf ("Writing index ordered by name");
   ByName (fp, fpOut);
   fclose (fpOut);

   rewind (fp);
   printf ("\n");

   if (!(fpOut = fopen ("sim-date.txt", "wt")))
      Error ("cannot open sim-date.txt");
   printf ("Writing index ordered by date");
   ByDate (fp, fpOut);
   fclose (fpOut);

   fclose (fp);
   printf ("\nDone.");
   return 0;
   }


/* ldisdbl.c - Stub for missing _LDisDouble_ in OpenWatcom v2 beta */
#pragma aux _LDisDouble_ "*";
int _LDisDouble_(long double *ld)
{
    (void)ld;
    return 1;
}

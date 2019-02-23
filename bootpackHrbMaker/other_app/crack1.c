void api_end();

void HariMain()
{
      *((char*)0x102600) = 0;
      api_end();
}
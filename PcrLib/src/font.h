


namespace pcrlib 
{
	class IFontRenderer
	{
	public:
		virtual void setString(const char *pStr, unsigned int x, unsigned int y) = 0;
		virtual void renderFont(ICBuffer *pDst, ICBuffer *pGlob) = 0;
		static IFontRenderer* getInstance();
		static void release(IFontRenderer ** ppIfr);
	}; //class FontRender

	

}// namespace pcrlib 
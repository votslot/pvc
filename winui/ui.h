
class UIOut {
public:
	virtual void OnFileOpen(const char *pPath) = 0;
	virtual void OnTestRun() = 0;;
	virtual void OnExit() = 0;
	virtual void OnPaint() = 0;

	void DispalyError(const char *pMsg);
	static void SetInstance(UIOut *pUiOut);
};


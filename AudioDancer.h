/**********************************
 * AudioDancer.h
 * Copyright © 1996 Steve Sprang
 * All Rights Reserved        
 *********************************/

#ifndef AUDIODANCER__H
#define AUDIODANCER__H

bool stream_function(void *, char *, long);

enum {
	mWAVES		=	'wave',
	mLINES		=	'line',	
	mSTATIC		=	'stat',
	mDANCE		= 	'danc'
};

/******************
 * SBlitView
 *****************/

class SBlitView : public BView {
	public:
						SBlitView(BRect, BBitmap *);
		virtual void 	Draw(BRect);
		virtual void 	Draw();
		
	private:
			BBitmap		*offscreen;
};


/******************
 * SAudioView
 *****************/

class SAudioView : public BView {
	public:
						SAudioView(BRect, BBitmap *);
		virtual			~SAudioView();				
		virtual void	AttachedToWindow();
				void   	SetBlitView(SBlitView *);
				void	SetFunction(ulong);
				bool    HandleData(char *, long);
				void	Waves();
				void	Lines();
				void	Static();
				void	Dancer();
				
	private:
				char 		*buffer;
				long		count;
				BBitmap		*offscreen;
				SBlitView	*blit;
				BRect		bound;
				
				int			*x, *y, numDots;
				int			*dx, *dy;
				rgb_color	*rgb;
				ulong		fn;
};

/******************
 * SWindow
 *****************/

class SWindow : public BWindow {
	public:
						SWindow(BRect, const char *);
		virtual			~SWindow();
		virtual	bool	QuitRequested();
		virtual void	MessageReceived(BMessage *);
				void	CreateMenus(BRect);
				bool	HandleData(char *, long);
	
	private:
				SAudioView 			*offview;
				BBitmap 			*offscreen;
				SBlitView			*blit;
				BAudioSubscriber 	*cd;
				BMenuBar			*menuBar;
};

/******************
 * SApplication
 *****************/

class SApplication : public BApplication {
	public:
				SApplication();
		void	AboutRequested();
				
	private:
				SWindow  *win;
};

#endif

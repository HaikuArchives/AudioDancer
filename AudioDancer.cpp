/**********************************
 * AudioDancer.cpp
 * Copyright © 1996 Steve Sprang
 * All Rights Reserved        
 *********************************/

#include "AudioDancer.h"

/******************
 * classless
 *****************/
				
main(int argc, char* argv[])
{
	SApplication *app;
	
	app = new SApplication();
	app->Run();
	
	delete(app);
	return(0);
}

bool stream_function(void *userData, char *buffer, long count)
{
	return ((SAudioView *)userData)->HandleData(buffer, count);
}

/******************
 * SApplication
 *****************/

SApplication::SApplication() : BApplication('adnc')
{
	BRect				bounds;
	screen_info			info;
	double				dx, dy;
	
	get_screen_info(&info);
	bounds.Set(0,0,256,272);
	
	dx = (info.frame.Width() * 0.5) - (0.5 * bounds.Width());
	dy = (info.frame.Height() * 0.5) - (0.5 * bounds.Height());
	
	bounds.OffsetTo(dx, dy);
	
	win = new SWindow(bounds, "AudioDancer v1.0");
	win->Show();
}

void SApplication::AboutRequested()
{
	BAlert	*about;
	
	char	name[80], license[80], email[80], all[240];
	
	all[0] = NULL;
	
	strcpy(name, "AudioDancer v1.0\nby Steve Sprang\nCopyright © 1996\nAll rights reserved.\n\n");
	strcpy(license, "Freeware.\n\n");
	strcpy(email, "e-mail: sprang@andrew.cmu.edu");
	
	strcat(all, name);
	strcat(all, license);
	strcat(all, email);
	
	about = new BAlert("", all, "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_INFO_ALERT);
	about->SetShortcut(0, B_ENTER);
	about->Go();
}

/******************
 * SWindow
 *****************/

SWindow::SWindow(BRect bounds, const char *title)
	: BWindow(bounds, title, B_TITLED_WINDOW, 
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
	BRect	menuBound;
	
	bounds.OffsetTo(B_ORIGIN);
	
	menuBound = bounds;
	menuBound.bottom = 16;
	CreateMenus(menuBound);
	AddChild(menuBar);
	
	offscreen = new BBitmap(bounds, B_COLOR_8_BIT, TRUE);
	offview = new SAudioView(bounds, offscreen);
	offscreen->AddChild(offview);
	
	bounds.InsetBy(0, 8);
	bounds.OffsetTo(0,17);
	blit = new SBlitView(bounds, offscreen);
	offview->SetBlitView(blit);
	AddChild(blit);
	
	cd = new BAudioSubscriber("");
	cd->SetADCInput(B_CD_IN);
	cd->Subscribe(B_ADC_STREAM, B_SHARED_SUBSCRIBER_ID, TRUE) < B_NO_ERROR;
	cd->EnterStream(NULL, TRUE, offview, stream_function, NULL, TRUE) < B_NO_ERROR;
}

void SWindow::CreateMenus(BRect bounds)
{
	BMenu		*display;
	BMenuItem	*newItem;
	
	// create the menu bar
	menuBar = new BMenuBar(bounds, "", B_FOLLOW_ALL, B_ITEMS_IN_ROW, FALSE);
		
	// create the "Display" menu
	display = new BMenu("Display", B_ITEMS_IN_COLUMN);
	
	// add items to "Display" menu
	newItem = new BMenuItem("Waves", new BMessage(mWAVES), 'A', NULL);
	display->AddItem(newItem);
	newItem->SetMarked(TRUE);
	newItem = new BMenuItem("Lines", new BMessage(mLINES), 'L', NULL);
	display->AddItem(newItem);
	newItem = new BMenuItem("Static", new BMessage(mSTATIC), 'S', NULL);
	display->AddItem(newItem);
	newItem = new BMenuItem("Dancer", new BMessage(mDANCE), 'D', NULL);
	display->AddItem(newItem);
	
	display->SetRadioMode(TRUE);
	menuBar->AddItem(display);
}

void SWindow::MessageReceived(BMessage *msg)
{
	offview->SetFunction(msg->what);
}

/* This doesn't quit for some reason */

bool SWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return TRUE;
}

SWindow::~SWindow()
{	
	cd->ExitStream(TRUE);
	delete cd;
	delete offscreen;
}

/*******************
 * SBlitView
 ******************/

SBlitView::SBlitView(BRect bound, BBitmap *b)
	: BView(bound, "", B_FOLLOW_NONE, B_WILL_DRAW)
{
	offscreen = b;
}

void SBlitView::Draw(BRect update)
{
	Draw();
}

void SBlitView::Draw()
{
	Window()->Lock();
	DrawBitmap(offscreen);
	Window()->Unlock();
}

/*******************
 * SAudioView
 ******************/

SAudioView::SAudioView(BRect bound, BBitmap *offscreen)
	: BView(bound, "", B_FOLLOW_ALL, B_WILL_DRAW)
{
	this->offscreen = offscreen;
	this->bound = bound;
	
	numDots = 16;
	
	x = new int[numDots];
	y = new int[numDots];
	dx = new int[numDots];
	dy = new int[numDots];
	rgb = new rgb_color[numDots];
	
	fn = mWAVES;
}

SAudioView::~SAudioView()
{
	delete x;
	delete y;
	delete dx;
	delete dy;
	delete rgb;
}

bool SAudioView::HandleData(char *buffer, long count)
{
	this->buffer = buffer;
	this->count = count;
	
	Window()->Lock();
	
	switch(fn) {
		case mWAVES:
			Waves();
			break;
		case mLINES:
			Lines();
			break;
		case mSTATIC:
			Static();
			break;
		case mDANCE:
			Dancer();
			break;
	}
	
	Window()->Unlock();
	blit->Draw();
	return TRUE;
}

void SAudioView::SetFunction(ulong fn)
{
	this->fn = fn;
	
	if(fn == mDANCE) {
		for(int i = 0; i < numDots; i++) {
			x[i] = y[i] = 128;
			dx[i] = dy[i] = 0;
			rgb[i].red = rgb[i].green = rgb[i].blue = 128;
		}
	}
}

void SAudioView::SetBlitView(SBlitView *blit)
{
	this->blit = blit;
}

void SAudioView::AttachedToWindow()
{
	SetViewColor(0,0,0);
}
	
void SAudioView::Waves()
{
	double 		average = 0.0;
	int			i,n;
	BPoint		start, end;
	rgb_color	color;
	
	SetHighColor(0,0,0);
	FillRect(bound);
	SetHighColor(255,255,255);
	
	color.red = color.blue = color.green = 255;
	
	BeginLineArray(256);
	
	for(n = 0; n < 16; n++) {
		average += buffer[n];
	}
	average /= 16.0;
	
	start.Set(0, average + 128);
	
	for(i = 16; i < count; i += 16) {
		average = 0.0;
		for(n = 0; n < 16; n++) {
			average += buffer[i + n];
		}
		average /= 16.0;
		end.Set(i / 16.0,average + 128);
		AddLine(start, end, color);
		start = end;
	}
	EndLineArray();
	Sync();
}

	
void SAudioView::Lines()
{
	BPoint		start, end;
	int			i, n;
	rgb_color	color;
	char		data[3];
	
	SetHighColor(0,0,0);
	FillRect(bound);
	
	start.Set(buffer[0] + 128, buffer[1] + 128);
	
	BeginLineArray(256);
	for(i = 0; i < count; i+= 128) {
		for(n = 0; n < 3; n++) {
			data[n] = buffer[i + 4 * n + 1] + 128;
			data[n] << 8;
			data[n] |= buffer[i + 4 * n] + 128;
		}
		
		color.red = data[0] % 255;
		color.green = data[1] % 255;
		color.blue = data[2] % 255;
		
		end.Set(buffer[i+14] + 128, buffer[i+15] + 128);
		SetHighColor(color);
		
		AddLine(start, end, color);
		
		start = end;
	}
	EndLineArray();
	Sync();
}

void SAudioView::Dancer()
{
	int 	avg = 0, avg2 = 0, val, val2, i, n;
	int		num = count / numDots;
	
	SetHighColor(0,0,0);
	FillRect(bound);
	
	for(i = 0; i < numDots-1; i++) {
		for(n = 0; n < num; n+=2) {
			if(n % 4 == 0) {
				avg += abs(buffer[i * num + n]);
			}
			else {
				avg2 += abs(buffer[i * num + n]);
			}
		}
		avg /= num / 2;
		avg2 /= num / 2;
		
		val = dx[i] - (avg % 10);
		val2 = dy[i] - (avg % 10);
		
		dx[i] = (avg % 10);
		dy[i] = (avg2 % 10);
		
		x[i] = ((255 / (numDots-1)) * i) + val + (255 / (numDots-1) / 2);
		y[i] += val2;
		
		if(x[i] > 255) {
			x[i] = 255;
		}
		else if(x[i] < 0) {
			x[i] = 0;
		}
		if(y[i] > 255) {
			y[i] = 255;
		}
		else if(y[i] < 0) {
			y[i] = 0;
		}
		
		rgb[i].red ^= abs(buffer[i * num]);
		rgb[i].green ^= buffer[i * num + num/2];
		rgb[i].blue ^= abs(buffer[i * num + num - 1]);
		
		SetHighColor(rgb[i]);
		FillEllipse(BRect(x[i] - 5, y[i] - 5, x[i] + 5, y[i] + 5));
	}
	Sync();
}
	
void SAudioView::Static()
{
	int	i;
	
	for(i = 0; i <= 16; i++) {
		offscreen->SetBits(buffer, count, i * count, B_COLOR_8_BIT);
	}
}
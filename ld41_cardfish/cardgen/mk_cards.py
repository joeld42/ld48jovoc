import os, sys
import re
import string
import math
import datetime
import random
import json

from PIL import Image, ImageDraw, ImageFont

class CardDef( object ):

	def __init__( self, title, artwork, text, reelPow ):

		self.title = title
		self.artwork = artwork
		self.text = text
		self.reelPow = reelPow



if __name__=='__main__':

	cardsImage = Image.new( "RGBA", (1024, 512), (255, 255, 255, 0) )
	draw = ImageDraw.Draw(cardsImage)

	cards = []
	cards.append( CardDef( "Lake Fish", "fishart1.png", "A pretty normal fish.", 2 ))
	cards.append( CardDef( "Smol Bass", "fishart1.png", "A diminuative bass. Not a lot of meat on it but still tasty.", 3 ))
	cards.append( CardDef( "High-Test Line", "tackleart1.png", "Line Tension +1.", 2 ))
	cards.append( CardDef( "Supple Gloves", "tackleart1.png", "Allowed Slack -1.", 2 ))
	cards.append( CardDef( "Cast!", "castart1.png", "CAST! Hook a fish from the lake!", 2 ))

	cardframe = Image.open("cardframe_normal.png")
	cardback = Image.open("cardback.png")

	CARD_HITE = 256
	scl = float(CARD_HITE) / cardframe.size[1] 
	CARD_WIDTH = int(cardframe.size[0]*scl)
	cardframe = cardframe.resize( (CARD_WIDTH,CARD_HITE), Image.ANTIALIAS ) 

	cardback = cardback.resize( (CARD_WIDTH,CARD_HITE), Image.ANTIALIAS ) 

	print "CARD ASPECT IS ", float(CARD_HITE) / float(CARD_WIDTH);
	print "CARD WIDTH IS ", CARD_WIDTH, (CARD_WIDTH/1024.0);

	fontName = "../gamedata/uiassets/Roboto-Medium.ttf"
	titleFont = ImageFont.truetype( fontName, 14)
	numberFont = ImageFont.truetype( fontName, 18)

	bodyFont = ImageFont.truetype( fontName, 12)

	# first do the card back
	cardsImage.paste( cardback, (0, 0) )

	currx = CARD_WIDTH
	curry = 0
	for cd in cards:

		fishart = Image.open( cd.artwork )
		scl = 141.0 / fishart.size[0]
		fishart = fishart.resize( (141, int(fishart.size[1]*scl)), Image.ANTIALIAS )
		cardsImage.paste( fishart, (currx+21,curry+27) )

		cardsImage.paste( cardframe, (currx, curry),mask=cardframe )
		draw.text( (currx+21,curry+134), cd.title, (72,31,7), titleFont )
		draw.text( (currx+9,curry+14), str(cd.reelPow), (58,175,35), numberFont )

		textwords = string.split(cd.text)
		currtexty = curry+154
		print textwords
		linewords = []		
		while len(textwords):
			teststr = string.join( linewords + [ textwords[0] ], " ")			
			linesz = draw.textsize( teststr, bodyFont )
			print "teststr", teststr, "linesz", linesz, len(textwords)
			if (linesz[0] > 137):
				# Doesn't fit, start a new line
				linetext = string.join(linewords," ")
				print "DRAW LINE", teststr
				draw.text( (currx+22,currtexty), linetext, (72,31,7), bodyFont )
				currtexty += linesz[1]
				linewords = []
			
			linewords.append( textwords.pop(0) )

		if len(linewords):
			draw.text( (currx+22,currtexty), string.join(linewords," "), (72,31,7), bodyFont )



		currx += CARD_WIDTH
		if (currx > CARD_WIDTH*4):
			currx = 0;
			curry += CARD_HITE		

	cardsImage.save("cardfish_cards.png")




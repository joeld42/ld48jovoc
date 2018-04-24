import os, sys
import re
import string
import math
import datetime
import random
import json

from PIL import Image, ImageDraw, ImageFont

CardType_CAST = 1
CardType_FISH = 2
CardType_TACKLE = 3
CardType_ACTION = 4

class CardDef( object ):

	def __init__( self, cardId, cardType, title, artwork, flavorText, actionText, reelPow, fishPoints, depth ):

		self.cardId = cardId
		self.cardType = cardType
		self.title = title
		self.artwork = artwork
		self.flavorText = flavorText.replace( '=', ',')
		self.actionText = actionText.replace( '=', ',')
		self.reelPow = reelPow
		self.fishPoints = fishPoints
		self.depth = depth

def stripQuotes( s ):
	return s.strip('"')

def doCardText( draw, text, font, currx, curry, texty, col ):
	xmarg  = 24
	textwords = string.split(text)
	currtexty = curry+154+texty
	leading = draw.textsize( "AMi!jgy")[1] + 2

	print textwords
	linewords = []		
	while len(textwords):
		teststr = string.join( linewords + [ textwords[0] ], " ")			
		linesz = draw.textsize( teststr, font )
		print "teststr", teststr, "linesz", linesz, len(textwords)
		if (linesz[0] > 137):
			# Doesn't fit, start a new line
			linetext = string.join(linewords," ")
			print "DRAW LINE", teststr
			draw.text( (currx+xmarg,currtexty), linetext, col, font )
			#currtexty += linesz[1]
			currtexty += leading
			linewords = []
		
		linewords.append( textwords.pop(0) )

	if len(linewords):
		draw.text( (currx+xmarg,currtexty), string.join(linewords," "), col, font )
		#currtexty += linesz[1]
		currtexty += leading

	return currtexty - (curry+154)

if __name__=='__main__':

	cardsImage = Image.new( "RGBA", (2048, 1024), (255, 255, 255, 0) )
	draw = ImageDraw.Draw(cardsImage)

	cards = []

	# Parse the card defs out of the game code:

	# cards.append( CardDef( "Lake Fish", "fishart1.png", "A pretty normal fish.", 2 ))
	# cards.append( CardDef( "Smol Bass", "fishart1.png", "A diminuative bass. Not a lot of meat on it but still tasty.", 3 ))
	# cards.append( CardDef( "High-Test Line", "tackleart1.png", "Line Tension +1.", 2 ))
	# cards.append( CardDef( "Supple Gloves", "tackleart1.png", "Allowed Slack -1.", 2 ))
	# cards.append( CardDef( "Cast!", "castart1.png", "CAST! Hook a fish from the lake!", 2 ))

	# TODO use the macro trick thing to do this
	cardTypeName = {
		"CardType_CAST" : CardType_CAST,
		"CardType_FISH" : CardType_FISH,
		"CardType_TACKLE" : CardType_TACKLE,
		"CardType_ACTION" : CardType_ACTION,
	}

	cardDefTokens = []
	inCardDef = False
	for line in open("../code/CardFishGame.cpp"):		
		#lsplit = string.split( string.strip(line), " ," )
		line = string.strip(line)

		line2 = ""
		instring = False
		for ch in line:
			if not instring:				
				if ch == " ":
					#print "CH",ch
					ch = "~"
			line2 = line2 + ch

			if ch=='"':
				instring = not instring

		lsplit = re.split( "[\~,]+", line2 )
		for token in lsplit:
			token = string.strip(token)

			if not inCardDef:
				if token == "AddCardDef(":
					print "inCardDef >> Now TRUE", token
					inCardDef = True
			elif token==");":

				inCardDef = False
				
				cardDef = []
				for tok in cardDefTokens:
					if (tok in ['', 'AddCardDef(']) or tok.startswith('/*'):
						continue
					cardDef.append( tok )

				cardDef = map( lambda x: stripQuotes(x), cardDef )				
				print "--"*10				
				print "cardDefTokens:", cardDef, len(cardDef)

				cardId, title, artwork, flavorText, actionText, cardType, reelPow, fishPoints, depth = cardDef
				card = CardDef( cardId, cardTypeName[cardType], title, 
							    artwork, flavorText, actionText, 
							    int( reelPow), int(fishPoints), int(depth) )
				cards.append( card )

				cardDefTokens = []
			else:
				print "inCardDef", inCardDef, "adding ", token
				cardDefTokens.append( token )				

	cardframe = Image.open("cardframe_normal.png")
	cardback = Image.open("cardback.png")

	vpicon = Image.open("vpicon.png")
	vpicon = vpicon.resize( (40, 40), Image.ANTIALIAS ) 

	CARD_HITE = 256
	scl = float(CARD_HITE) / cardframe.size[1] 
	CARD_WIDTH = int(cardframe.size[0]*scl)
	cardframe = cardframe.resize( (CARD_WIDTH,CARD_HITE), Image.ANTIALIAS ) 

	cardback = cardback.resize( (CARD_WIDTH,CARD_HITE), Image.ANTIALIAS ) 

	print "CARD ASPECT IS ", float(CARD_HITE) / float(CARD_WIDTH);
	print "CARD WIDTH IS ", CARD_WIDTH, (CARD_WIDTH/2048.0);

	fontName = "Gora-Rough.ttf"
	actionFontName = "Roboto-Medium.ttf"
	flavorFontName = "LazyFox.ttf"

	titleFont = ImageFont.truetype( fontName, 14)
	numberFont = ImageFont.truetype( fontName, 18)
	vpfont = ImageFont.truetype( fontName, 24)
	tinyFont = ImageFont.truetype( actionFontName, 10 )
	tinyFont2 = ImageFont.truetype( actionFontName, 9 )

	bodyFont = ImageFont.truetype( actionFontName, 12)
	flavorFont = ImageFont.truetype( flavorFontName, 12)


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

		texty = 0
		SPC = 8
		if cd.flavorText:
			texty = doCardText( draw, cd.flavorText, flavorFont, currx, curry, texty, (72,31,7) )
			texty += 8

		if cd.actionText:
			texty = doCardText( draw, cd.actionText, bodyFont, currx, curry, texty, (14,78,138) )
			texty += 8

		if (cd.cardType == CardType_FISH):
			cardsImage.paste( vpicon, (currx+127, curry+203), mask=vpicon )
			draw.text( (currx+139, curry+212), str(cd.fishPoints), (255,255,255), vpfont );

			# draw.rectangle( (currx+115, curry+134, currx+160, curry+146 ),
			# 	fill=(35,94,175), outline=(36,61,128) )
			# scooch = 3
			# font = tinyFont
			# if cd.depth < 6:
			# 	depthText = "Shallow"
			# else:
			# 	depthText = "Deepwater"
			# 	scooch = -4
			# 	font = tinyFont2

			# draw.text( (currx+120+scooch, curry+136), depthText, (120,190,230), font );



		currx += CARD_WIDTH
		if (currx > CARD_WIDTH*10):
			currx = 0;
			curry += CARD_HITE	

	cardsImage.save("cardfish_cards.png")




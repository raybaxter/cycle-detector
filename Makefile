cycle_detector: cycle_detector.c
	gcc -Wall cycle_detector.c -o cycle_detector

web: cycle_detector.html

test: 

cycle_detector.html: cycle_detector.c header.html footer.html	
	sed "s/TITLE/Graph Cycle Detector/" header.html > cycle_detector.html
	sed -e "s/	/  /g" -e "s/</\&lt;/g" cycle_detector.c >> cycle_detector.html
	cat footer.html >> cycle_detector.html
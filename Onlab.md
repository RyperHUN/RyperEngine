Sourcek eddig:
Karakter animacio:
	-Assimp Scene struct: http://assimp.sourceforge.net/lib_html/structai_scene.html
	-Assimp reszletes leiras adatokrol: http://assimp.sourceforge.net/lib_html/data.html
	-Kar Animacio D3D ben elmagyarazva: http://mathinfo.univ-reims.fr/image/dxMesh/extra/d3dx_skinnedmesh.pdf
	-OpenGL Animacio elmagyarazva[Eleg hulyen]: http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html 
Component Based:
	-RandyGaul [Tul optimalizalt] = http://www.randygaul.net/2013/05/20/component-based-engine-design/
	-GameProg patterns  http://gameprogrammingpatterns.com/component.html
	
Kerdesek:
	-Miert lenghet ki egy kicsit a jobb (szembol bal) laba a karatkernek?
		-Nyers adat bongesz. -> Megnez az elforgatasokat az ellentetes labra.
	-Effect framework mire jo? Pros cons? 
		(Nekem ugyjott le hogy #include tud segiteni kicsit ha sok helyen duplikalt kod lenne)
		-Modositasz valamit a meneten-> Render state -> Akkor nem kell ujraforditani.
		-Uniform Textura bekotesnel van -> Nem kell megmondani melyik regiszterhez tartozik.
	-Cook-Torrance modell mit jelent?
		-BRDF Helyett gondoljuk azt hogy ez a felulet kicsike kis siklapkakbol allnak (Microfacets)
		 Ha ezek a kis lapkak nagyjabol ugyanugy allnak. Teljesen osszevissza: Nem diffuz, ha egybe all akkor diffuz.
		 Van egy ilyen roughness : Mennyire allnak osszevissza. -> Ennek a fuggvenyebe vezeti le.
		 (Phong nem szimmetrikus -> kimeno es a bemeno iranyt nem ugyanazt a szamot kapom (Reciprocitas))
		 Nem szivas kiszamolni, de nem nez ki jobban.
	-Phong nal a reflect el szamitott spekularis miert nem jo?
		-Phong azt gondolja hogy a fele elnyelodik, de valosagban jobban visszaverodik. [Elkuldom]
	-Shadow volume? Mit jelent? Ez a john carmackos megoldas Stencillel?
		-Kiszamoljuk azt a testet amin belul arnyek van.
		-Megkeres silouette el. -> Geometry shaderbe eltudjuk donteni, amelyik silouette el, azt kitodjuk huzni es tudunk
		belole csinalni egy quadot.
		- Aztan ezekbe el kell doonteni hogy benne van-e a feluletbe amit kirendeltuk.
		- Stencil testet konfiguraljuk be ugy hogy?
		Hatranyai: -Nem tud soft shadowt. (Nem tudjuk elkenni)
				   -Sok pixel mire kirajzoljuk.
		 -> Shadow mapbe: -Itt a bias ottvan, meg az artifact.
						  -Soft shadow: Melyseg buffert elkenjuk nem a legjobb
									   -Percentage Closure Filtering (Kozelebb levok szazalekara szurunk)
												-> Szomszedok atlagolasa es az ad meg egy sotetseget.
								       -Van egy masik: Variance Shadow Map (Csebisevet hasznalja)
	-Quaterniobol hogyan lehet kiszedni az iranyt? 
	- Offscreen rendering, hogyan kene jol?
		//-> Folyamatosan uj texturat generalok -> igy nem kell szinkronizalni -> gyorsabb.
		- Framenkent ne legyen eroforras letrehozas !!!!!!!!![TODO]
	-Nap megjelenitese? Szamitogepes grafikabol volt szo a vec4 rol, ha az utso koord 0 akkor idealis pont… [Email -> Keplet]
		-> Vegtelen messzi van levagodik -> Amikor a hatteret rajzolja az ember, akkor beleveszi a napot is.
	
	-Flat shading?
	-Hova kene eljutni a vegere? Tehat mi az amit eltudok adni mint szakdoga?
	-Szerintem jovoheten nemjovok konzultalni, de tobbet haladnek, mikkel haladjak?
	-Konyv ajanlas?
	
	-//TODO Tone map akar tamas jegyzetebol
	
Terv:
- Frustum Culling
- Animacio csak gombnyomasra?
- 
%TODO Irni, megoldani viz renderelest [https://www.youtube.com/watch?v=HusvGeEDU_U&list=PLRIWtICgwaX23jiqVByUs0bqhnalNTNZh]
%TODO Irni PhysX ről.
%TODO Irni font renderingrol???
%TODO Irni Extra OpenGL Effektek megvalositva: Anti Aliasing, Post Processing(Tone Mapping, gamma korrekcio) 
%TODO Irni kamerarol?
	%TODO Megold: Lens Flare effect, Render to cube map texture (For dynamic environment mapping)
	%TODO PostProcess more effekt: Bloom effekt, Gaussian Blur.
	%TODO Rendes nap megjelnites texturabol stb, es rarajzol skyboxra.
	%TODO Particle effects
	%TODO Normal Mapping
	%TODO Render GUI
	%TODO Fog tavoli objektumokhoz, ne legyen eles valtas!!!
	%TODO Profiler tamas tipp: ApiTrace
%TODO Debug Context OpenGL be
%TODO gl::DebugOutput::AddErrorPrintFormatter([](gl::ErrorMessage) {assert(false);});
%Meg ugye ezt a hívást nem árt #if OGLWRAP_DEBUG-ba tenni, mert különben nincs DebugOutput osztály
%Profiling: https://www.reddit.com/r/opengl/comments/6n7sli/how_to_find_bottleneck_in_opengl_application/

Sourcek eddig:
	-Learn OpenGL [Bennevan]
	-Thinmatrix Youtube https://www.youtube.com/channel/UCUkRj4qoT1bsWpE_C8lZYoQ
Karakter animacio:
	-Assimp Scene struct: http://assimp.sourceforge.net/lib_html/structai_scene.html [BENNE]
	-Assimp reszletes leiras adatokrol: http://assimp.sourceforge.net/lib_html/data.html [BENNEVAN]
	-Kar Animacio D3D ben elmagyarazva: http://mathinfo.univ-reims.fr/image/dxMesh/extra/d3dx_skinnedmesh.pdf [NINCS BENNE]
	-OpenGL Animacio elmagyarazva[Eleg hulyen, de megis jol]: http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html  [BENNEVAN]
Component Based:
	-RandyGaul [Tul optimalizalt] - http://www.randygaul.net/2013/05/20/component-based-engine-design/
	-GameProg patterns - http://gameprogrammingpatterns.com/component.html
Frustum Culling:
	-http://www.lighthouse3d.com/tutorials/view-frustum-culling/view-frustums-shape/ [BENNEVAN]
	- TODO Separating Axis Theorem ( By Tamas )
Arnyekok:
	-Shadow mapping -> Errol lehetne nagyon rizsazni, tobbfele softshadow megoldasrol foleg
	-http://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf [Nincs Benne]
	-http://developer.download.nvidia.com/presentations/2008/GDC/GDC08_SoftShadowMapping.pdf [BENNEVAN]
Minecraft:
	Procedural Gen:
		-http://codeflow.org/entries/2010/dec/09/minecraft-like-rendering-experiments-in-opengl-4/ [BENNEVAN] TODO Procedural gen
		-https://notch.tumblr.com/post/3746989361/terrain-generation-part-1
	-Array Texture -> Utananez textura feltolteshez [BENNEVAN]
		https://sites.google.com/site/john87connor/texture-object/tutorial-09-6-array-texture [BENNEVAN]

Kerdesek:
	- PhysX Particle systemje az pontosan mit csinal? Mert ugye en fogom megjeleniteni, o csak a mozgast szimulalja? [Hardcore]
		/Utkozeseket is szamol a particlek kozott meg a feluletek kozott. (PL a fust ne menjen at a falon)
		/Leontod vodor vizzel az autot akkor szepen lefolyik.
	- Mi az a scenequery? ezt nyomja a PhysX be a shapeknel. Egy shape lehet intersect test, trigger volume vagy ez.
		/??????
	- Mi a kulonbseg az egyes tipusok kozott: Static (Gondolom ez lenne minecraftba a blokk), Dynamic (Nyilvesszo), Kinematic (Karakter)
		- Kinematic character: O hat mas objektumokra felre tudja lokni a hordot stb. De hogy o hogyan mozogjon azt le kell kodolni.
		- Move function: 
	- Azt beszeltuk konzultacion hogy kis spherekkel kene kozeliteni az egyes csontokat,
	  nem lenne eleg ha az egesz mest kozelitem egy Konvex Burokkal?
		- Csontonkent egy capsula. Csont trafok alapjan megtudom mondani hova kell mennie a kapszulaknak. (Ez a rongybabahoz)
			-> Mayahoz plugin PhysX. ez legeneralja a kapsulakat. (NXU kiterjesztesu fajl.) [Ez egy scene lesz es betudom peldanyositani]
	- Kinematic Controllerbe azt olvastam hogy nekem kell megoldani a gravitációt, de azt hogyan?
		/Raycastolni kene es meg kell neznem hogy bele megy-e vagy nem, de valoszinuleg query-t kell hasznalni
		/Sweep query karakter nekimegy a falnak vagy sem
	- Mi a kulonbseg a ride & slide kozott es a sima ride kozott?
		/
	- Hogyan vizualizaljam az objektumokat en is Grafikaban??? (Mint kb ahogy a PX Visual debugger csinalja)
	- Miért nem konzisztens a kép a PhysX es cuccal?
		/
	- Vélemény az eddig irtakról? Mikor célszerű kódrészletet beszúrni?
	- Miért lassú a textúra váltás? Nem csak egy uniform feltöltése az egész hogy melyik textúrát használja?
		Ami a nagyon lassu az a Cntext meg a shader valtas. A textura valtas nem olyan lassu, de ha 1 rajzolashivassal eltudod intezni
		textura array, akkor ezzel a rajzolas hivason nyersz.
		(Textura cache - ha egyszer innen egyszer onnan olvasol nemjo, de a textura cache lenyege az hogy a szomszedos pixeleket behuzza)
	
	%TODO PCF Irni beepitett dologrol
	
Kerdesek:
	-Megkerdez hogy mihez vegyem fel a szakdogat, mit irjak, mi legyen a tema cime?
		-[Rendering Engine] vs Minecraft Jatek.
		- IIT-s szakdoga de ehhez kell papirmunka hogy feltudjam venni.
		
	-Szakdogaba ehhez vigan irhatok dolgokat?
		-Gondolom ugy kozelitsem meg ezeket hogy valamihez kepest osszehasonlitom, hogy ez ennel jobb de pl ennel rosszabb, es akar egy
		 tablazatot is belerakhatok ilyen sebessegekkel stb.
		 
-Elozo modszer, Sajat resz mit csinaltam, utana a felhasznalt technologia, ertekeles osszehasonlitas.		 
	

	- Hogyan hasznalj profilert? 
		[-Masik program]
		-Hogyan talalom meg hol a bottleneck
	-Megoldottam az ObjectPickingest ahogy emlitetted, nem olvasok readPixelt, viszont ez nem ertem miert mukodik??
		-  A z koordinata csak a hosszat befojasolja.
	-Texture Buffer mire jo?
		- TBuffer -> 2 fajta memoria, ez az egyik. Az a kulonbseg milyen modon eri el a grafikus kartya, olyan mintha shader
			constans. Textura memoria olvasas, nem globalis. Tobb adat elfer benne. Milyen eleres hatekony. Ha ossze vissza kell
			belole adat akkor ez jobb (Random eleres hatekonyabb)
			PL:[Mesh RayTrace]
		- CBuffer -> Uniform -> GLobal memoria olvasas. -> Ide azt erdemes rakni ami mindig kell. Pl Matrix. (Random eleres nem hatekony)
	-Hogyan kell Transparent renderinget csinalni?
		-Minecraftba ottvan egy virag az 2 quad X alakba, ezt hogyan lehetne szepen? Sehogy? Csak discardal? Az nem lassu?
		-PL X quad es mit csinaljak, legyen 4 quad.
			-> shader nem adja ki a szint, hanem fogjuk a fragmenseket, es gyujtsok ossze mindegyiket.
			Order Independent Transparency.
			-> Lancolt lista
			-> Pixelenkent rendezes
			De ze nem ultimate modszer -> Ossze vissza helyezkednek el. (Cache nem jo)
	[- Uniform buffer object mikor jo pl? (PL kozos kamera mindegyik shaderbe)]
	- Utkozes detektalas hogyan?
		-Valami kozelito geometria kell:
			-Befoglalo gomb, Befoglalo doboz, 
			Nagyon pontosan konvex polieder minden egyes csontra.
			-> Rigging body -> Ebbol akkor lehetne rongybaba
			-PhysX, Bullet.
		-Ha detektalom utana jon gondolom ez a rugos dolog amit beszeltunk, de ezt minden utkozesnel?
			-PhysX be nem kell ilyen.
			-PhysX nelkul meg meg konnyu lekodolni.
			[Minel jobban osszeer annal jobban elugranak + Csillapitas]
			-Fal utkozesnel ki szoktak dobni a fizikat!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			-Karakter mozgasat kulon kod irja, Ha mesz a hordo felborul, de a hordo nem hat vissza rad.
	-Texturat hogyan kene feltolteni Blockokhoz? 
		-Itt az a zavaro hogy van olyan block amihez tobb texture tartozik pl Fu
		-[Tudni kell hogy melyik szelet kell: uniform vagy atribute. Instance -> 6 db lapra melyiket kell rakni]
	-Valahogy kinyerheto egy quaterniobol hogy egy adott tengely menten mennyire van elforgatva?
		- Karakterhez kene pl Y tengely menten mennyire fordult el
	-Particle System?
		->Grafikus Jatekok fejlesztese -> Hogyan lehet reszecskeket kirajzolni rendezni stb.
		->Ne legyen konstans a reszecskek szama, fel is robbanhassanak ilyenek.
	-Valamilyen konyv amit tudnal ajanlani?
		->Game Engine Architectures. [Inkabb altalanosabb]
		
	[OpenGL Konyv, NSight helyett masik]
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
	-Hova kene eljutni a vegere? Tehat mi az amit eltudok adni mint szakdoga? -> Barmit elkezdhetem irni
	-Szerintem jovoheten nemjovok konzultalni, de tobbet haladnek, mikkel haladjak?
	-Konyv ajanlas?
	
	-//TODO Tone map akar tamas jegyzetebol
	
	
Terv:
- Frustum Culling - KEEEESZ
- TODO Arnyekok mindennel. - KESSSZ
- TODO Add defs.h -> sok typedef

- Animacio csak gombnyomasra.
- TODO Add namespace
- TODO Lecserel tamas wrapperjere!! 


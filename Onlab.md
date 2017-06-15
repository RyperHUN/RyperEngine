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
	-Effect framework mire jo? Pros cons? 
		(Nekem ugyjott le hogy #include tud segiteni kicsit ha sok helyen duplikalt kod lenne)
	-Cook-Torrance modell mit jelent?
	-Phong nal a reflect el szamitott spekularis miert nem jo?
	-Shadow volume? Mit jelent? Ez a john carmackos megoldas Stencillel?
	-Quaterniobol hogyan lehet kiszedni az iranyt? 
		->Lattam egy kamerat amit quaterniokkal oldottak meg, a pozicio ugye fix, de a kamera iranyat abbol hogy kapom meg?
		->Illetve view es projection?
	-Flat shading? Hogyan lehet ilyet? Mostanaba nagyon meno ez a technika szerintem [Flat shading kep included]#
	- Offscreen rendering, hogyan kene jol?
	
	-Szerintem jovoheten nemjovok konzultalni, de tobbet haladnek, mikkel haladjak?
	-Hova kene eljutni a vegere? Tehat mi az amit eltudok adni mint szakdoga?
	
1) Mi az az offset Matrix?
	bone -> localba visz.
	Bor feluleti pontjat hogyan kapod meg.

2) aiNode -> Fa hierarcia oke
	Transform itt mit jelent? Csont sajat transzformacioja? Minden csontnak van transzformacios matrixa
	a szulo csonthoz kepest.
	
Offset matrix: Csonthoz kepest feluleti pont transzformaciojat irja le. Csonthoz kepest hol a boron a pont.
AINode Trafo: Csontok poziciojara vonatkozik.

Bone -> Local
  -Rekurziv offset matrixok szorzata, ha nincs animalva.
  -

localTransform = Bone megfelelo helyre
GlobalTransform = parent_transform * local -> Bone space bol atvisz localba.

Final transform = Riggelt pozicioba adott mesh hol van.

RootNode ra GlobalTransform = Ebbe van benne hogy nem minden modellezobe van Y kordinata felfele.
Ebbe van beleegetve hogy hogyan kell ezt visszahakolni. Ha az egesz trafot megszorzod ezzel, akkor jo lesz!

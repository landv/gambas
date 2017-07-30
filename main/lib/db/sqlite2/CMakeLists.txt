gb_component(NATIVE gb.db.sqlite2
    REQUIRED_PACKAGES SQLite2
    SOURCE_LIST main.cpp dataset.cpp qry_dat.cpp sqlitedataset.cpp stringhelper.cpp)
gb_disable_component(gb.db.sqlite2) #Todo: fix compilation

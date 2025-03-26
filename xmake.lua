add_rules("mode.debug", "mode.release")

target("ZoneTracer")
    add_rules("qt.widgetapp")
    set_kind("binary")
    add_cxflags("-mavx2 -fPIC")
    add_files("QtWidgetsApplication1.qrc")
    add_files("columnfilter.h")
    add_files("dialog/configdialog.h")
    add_files("LineEditFilter.h")
    add_files("multiThread.h")
    add_files("QtWidgetsApplication1.h")
    add_files("topicData/ZoneMasterSomeipDataCallingListener.h")
    add_files("topicData/ZoneMasterCanMessageDataListener.h")
    add_files("topicData/ZoneMasterSomeipDataEthFrameListener.h")
    add_files("topicData/ZoneMasterSomeipDataSDListener.h")
    add_files("topicData/ZoneMasterCanParserListener.h")
    add_files("topicData/ZoneMasterLinParserDataListener.h")
    add_files("topicData/ZoneMasterLinMessageDataListener.h")
    add_files("topicData/ZoneMasterSomeipDataPackageListener.h")
    add_files("topicData/ZoneMasterSomeipDataStateListener.h")
    add_files("treemodel.h")
    add_files("usagedialog.h")
    add_files("topicData/ZoneMasterCanMessageDataSubscriber.cpp")
    add_files("topicData/ZoneMasterCanMessageDataSubscriber.cpp")
    add_files("topicData/ZoneMasterCanParserSubscriber.cpp")
    add_files("topicData/ZoneMasterData.cxx")
    add_files("topicData/ZoneMasterDataPubSubTypes.cxx")
    add_files("topicData/ZoneMasterLinMessageDataSubscriber.cpp")
    add_files("topicData/ZoneMasterLinParserDataSubscriber.cpp")
    add_files("topicData/ZoneMasterSomeipDataSubscriber.cpp")
    add_files("columnfilter.cpp")
    add_files("dialog/configdialog.cpp")
    add_files("multiThread.cpp")
    add_files("TraceTreeWidgetItem.cpp")
    add_files("treeitem.cpp")
    add_files("treemodel.cpp")
    add_files("usagedialog.cpp")
    add_files("QtWidgetsApplication1.cpp")
    add_files("main.cpp")
    add_includedirs(".")
    add_includedirs("/usr/include")
    add_includedirs("/usr/local/include")
    add_includedirs("/usr/local/include/fastrtps/")
    add_includedirs("/usr/local/include/fastrtps/common/")
    add_includedirs("/usr/local/include/fastrtps/utils/")
    add_includedirs("/usr/local/include/fastcdr/")
    add_includedirs("/usr/local/include/fastcdr/cdr/")
    add_includedirs("/usr/local/include/fastcdr/exceptions/")
    -- add_includedirs("/usr/include/libdrm")

    local qt_path = "/opt/Qt5.14.2/5.14.2/gcc_64"
    add_includedirs(
	path.join(qt_path, "include"),
	path.join(qt_path, "include/QtCore"),
	path.join(qt_path, "include/QtWidgets"),
	path.join(qt_path, "include/QtGui"),
	path.join(qt_path, "mkspecs/linux-g++")
    )

    add_links("Qt5Widgets",
	 "Qt5Core",
	 "Qt5Gui",
	 "fastrtps",
	 "fastcdr",
	 "pthread",
	 "foonathan_memory-0.7.3")

    add_linkdirs(
	path.join(qt_path, "lib"),
	"/usr/local/lib",
	"/usr/lib"
    )

    after_build(function (target)
	import("core.project.config")
	local targetfile = target:targetfile()
	local src_dir = "."
	local dest_dir = "app"
	local inst_dir = "installer"

	os.tryrm(dest_dir)
	os.mkdir(dest_dir)
	os.tryrm(inst_dir)
	os.mkdir(inst_dir)

	local files_to_copy = {
	    targetfile,
	    path.join(src_dir, "Zone.desktop"),
	    path.join(src_dir, "ZoneTracer.png")
	}

	for _, file in ipairs(files_to_copy) do
	    os.cp(file, path.join(dest_dir, path.filename(file)))
	end

	os.cd(dest_dir)
	os.exec("linuxdeployqt ZoneTracer -appimage")

  -- make the installer folder
	os.exec("cp ../settings.ini ../installer/")
	local files = os.match("*.AppImage")
	for _, source_file in ipairs(files) do
	    print(source_file)
            local file_name = path.filename(source_file)
            local dest_file = path.join("..", inst_dir, "ZoneTracer.AppImage")
            os.cp(source_file, dest_file)
        end

    end)

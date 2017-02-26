{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 7,
			"minor" : 3,
			"revision" : 2,
			"architecture" : "x64",
			"modernui" : 1
		}
,
		"rect" : [ 325.0, 78.0, 1225.0, 934.0 ],
		"bglocked" : 0,
		"openinpresentation" : 0,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Arial",
		"gridonopen" : 1,
		"gridsize" : [ 15.0, 15.0 ],
		"gridsnaponopen" : 1,
		"objectsnaponopen" : 1,
		"statusbarvisible" : 2,
		"toolbarvisible" : 1,
		"lefttoolbarpinned" : 0,
		"toptoolbarpinned" : 0,
		"righttoolbarpinned" : 0,
		"bottomtoolbarpinned" : 0,
		"toolbars_unpinned_last_save" : 0,
		"tallnewobj" : 0,
		"boxanimatetime" : 200,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 0.0,
		"description" : "",
		"digest" : "",
		"tags" : "",
		"style" : "",
		"subpatcher_template" : "",
		"boxes" : [ 			{
				"box" : 				{
					"id" : "obj-24",
					"linecount" : 3,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 874.0, 856.0, 150.0, 47.0 ],
					"style" : "",
					"text" : "Example of interfacing hedrot with IRCAM Spat - 1 dry source"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 11.0,
					"id" : "obj-19",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 1073.0, 428.75, 75.0, 21.0 ],
					"style" : "",
					"text" : "prepend load"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-20",
					"maxclass" : "dropfile",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"patching_rect" : [ 1073.0, 393.25, 110.0, 30.0 ]
				}

			}
, 			{
				"box" : 				{
					"bgmode" : 0,
					"border" : 0,
					"clickthrough" : 0,
					"enablehscroll" : 0,
					"enablevscroll" : 0,
					"id" : "obj-25",
					"lockeddragscroll" : 0,
					"maxclass" : "bpatcher",
					"name" : "spat.hrtf.selection.maxpat",
					"numinlets" : 1,
					"numoutlets" : 2,
					"offset" : [ 0.0, 0.0 ],
					"outlettype" : [ "", "" ],
					"patching_rect" : [ 1031.0, 278.75, 160.0, 95.0 ],
					"viewvisibility" : 1
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-18",
					"maxclass" : "newobj",
					"numinlets" : 3,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 887.0, 397.25, 76.0, 22.0 ],
					"style" : "",
					"text" : "pack 0. 0. 0."
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-17",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 887.0, 427.0, 119.0, 22.0 ],
					"style" : "",
					"text" : "listener rpy $3 $2 $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-15",
					"maxclass" : "live.gain~",
					"numinlets" : 2,
					"numoutlets" : 5,
					"outlettype" : [ "signal", "signal", "", "float", "list" ],
					"parameter_enable" : 1,
					"patching_rect" : [ 874.0, 557.0, 48.0, 136.0 ],
					"saved_attribute_attributes" : 					{
						"valueof" : 						{
							"parameter_longname" : "live.gain~[1]",
							"parameter_shortname" : "live.gain~[1]",
							"parameter_type" : 0,
							"parameter_mmin" : -70.0,
							"parameter_mmax" : 6.0,
							"parameter_initial" : [ 0.0 ],
							"parameter_unitstyle" : 4
						}

					}
,
					"varname" : "live.gain~"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-13",
					"maxclass" : "ezdac~",
					"numinlets" : 2,
					"numoutlets" : 0,
					"patching_rect" : [ 874.0, 730.0, 45.0, 45.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"args" : [ 1 ],
					"bgmode" : 0,
					"border" : 1,
					"clickthrough" : 0,
					"enablehscroll" : 0,
					"enablevscroll" : 0,
					"id" : "obj-4",
					"lockeddragscroll" : 0,
					"maxclass" : "bpatcher",
					"name" : "spat.multiinputs~.maxpat",
					"numinlets" : 1,
					"numoutlets" : 1,
					"offset" : [ 0.0, 0.0 ],
					"outlettype" : [ "signal" ],
					"patching_rect" : [ 874.0, 179.0, 201.0, 54.0 ],
					"viewvisibility" : 1
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 18.0,
					"id" : "obj-6",
					"linecount" : 2,
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 3,
					"outlettype" : [ "signal", "signal", "" ],
					"patching_rect" : [ 874.0, 474.0, 349.0, 49.0 ],
					"saved_object_attributes" : 					{
						"parameter_enable" : 0
					}
,
					"style" : "",
					"text" : "spat.pan~ @numinputs 1 @numoutputs 2 @type binaural"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 11.0,
					"id" : "obj-48",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 625.0, 331.75, 29.0, 21.0 ],
					"style" : "",
					"text" : "thru"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 11.0,
					"id" : "obj-42",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "source" ],
					"patching_rect" : [ 140.0, 564.0, 29.0, 21.0 ],
					"style" : "",
					"text" : "thru"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 16.0,
					"id" : "obj-40",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 4,
					"outlettype" : [ "source", "speaker", "listener", "" ],
					"patching_rect" : [ 140.0, 500.0, 114.0, 26.0 ],
					"saved_object_attributes" : 					{
						"parameter_enable" : 0
					}
,
					"style" : "",
					"text" : "spat.transform"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 11.0,
					"id" : "obj-46",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 686.5, 233.75, 66.0, 19.0 ],
					"presentation" : 1,
					"presentation_rect" : [ 396.5, 41.0, 66.0, 19.0 ],
					"style" : "",
					"text" : "ROTATION"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 11.0,
					"format" : 6,
					"id" : "obj-33",
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 765.0, 260.0, 50.0, 21.0 ],
					"presentation" : 1,
					"presentation_rect" : [ 475.0, 67.25, 50.0, 21.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 11.0,
					"id" : "obj-34",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 765.0, 290.0, 40.0, 21.0 ],
					"style" : "",
					"text" : "roll $1"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 11.0,
					"format" : 6,
					"id" : "obj-35",
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 695.0, 260.0, 50.0, 21.0 ],
					"presentation" : 1,
					"presentation_rect" : [ 405.0, 67.25, 50.0, 21.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 11.0,
					"id" : "obj-36",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 695.0, 290.0, 49.0, 21.0 ],
					"style" : "",
					"text" : "pitch $1"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 11.0,
					"format" : 6,
					"id" : "obj-37",
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 625.0, 260.0, 50.0, 21.0 ],
					"presentation" : 1,
					"presentation_rect" : [ 335.0, 67.25, 50.0, 21.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 11.0,
					"id" : "obj-38",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 625.0, 290.0, 45.0, 21.0 ],
					"style" : "",
					"text" : "yaw $1"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 11.0,
					"id" : "obj-11",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"patching_rect" : [ 140.0, 160.0, 56.0, 21.0 ],
					"style" : "",
					"text" : "loadbang"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 11.0,
					"id" : "obj-8",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"patching_rect" : [ 20.0, 130.0, 56.0, 21.0 ],
					"style" : "",
					"text" : "loadbang"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-14",
					"maxclass" : "button",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"patching_rect" : [ 140.0, 190.0, 20.0, 20.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 11.0,
					"id" : "obj-12",
					"linecount" : 4,
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 20.0, 160.0, 90.0, 58.0 ],
					"style" : "",
					"text" : "numsources 1, numspeakers 4, showaperture 0, zoom 0.4"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 11.0,
					"id" : "obj-10",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"patching_rect" : [ 315.833344, 520.0, 89.0, 21.0 ],
					"style" : "",
					"text" : "routepass zoom"
				}

			}
, 			{
				"box" : 				{
					"aperture" : [ 54.768356 ],
					"aperturecolor" : [ 1.0, 1.0, 1.0, 0.619608 ],
					"areasmonitoring" : [ 0 ],
					"autozoom" : [ 0 ],
					"backgroundcolor" : [ 0.7, 0.7, 0.7, 0.7 ],
					"backgroundimage" : [ "none" ],
					"backgroundimageangle" : [ 0.0 ],
					"backgroundimageopacity" : [ 1.0 ],
					"backgroundimagequality" : [ "medium" ],
					"backgroundimagescale" : [ 1.0 ],
					"backgroundimagexoffset" : [ 0.0 ],
					"backgroundimageyoffset" : [ 0.0 ],
					"circularconstraint" : [ 0 ],
					"dashedgrid" : [ 0 ],
					"defer" : [ 0 ],
					"display" : [ 1 ],
					"format" : [ "aed" ],
					"globalproportion" : [ 0.1 ],
					"gridcolor" : [ 1.0, 1.0, 1.0, 1.0 ],
					"gridlines" : [ 3 ],
					"gridmode" : [ "circular" ],
					"gridspacing" : [ 1.0 ],
					"id" : "obj-3",
					"jspainterfile" : [ "" ],
					"layout" : [ "leftright" ],
					"listenereditable" : [ 0 ],
					"listenerpitch" : [ 0.0 ],
					"listenerposition" : [ 0.0, 0.0, 0.0 ],
					"listenerproportion" : [ 0.15 ],
					"listenerroll" : [ 0.0 ],
					"listeneryaw" : [ 0.0 ],
					"maxclass" : "spat.viewer.embedded",
					"name" : [ "1" ],
					"numanchors" : [ 0 ],
					"numangulardivisions" : [ 8 ],
					"numareas" : [ 0 ],
					"numinlets" : 1,
					"numoutlets" : 7,
					"numsources" : [ 1 ],
					"numspeakers" : [ 4 ],
					"orientationmode" : [ "yawconstraint" ],
					"outlettype" : [ "source", "speakers", "source", "", "listener", "", "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 140.0, 599.0, 520.0, 288.0 ],
					"presentation" : 1,
					"presentation_rect" : [ 560.0, 160.0, 520.0, 288.0 ],
					"radius" : [ 1.0 ],
					"radiusconstraint" : [ 0 ],
					"rightclicklock" : [ 0 ],
					"shoeboxcorners" : [ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 ],
					"showanchors" : [ 0 ],
					"showanchorslabel" : [ 0 ],
					"showangulardivisions" : [ 0 ],
					"showaperture" : [ 0 ],
					"showareas" : [ 0 ],
					"showbackgroundimage" : [ 1 ],
					"showfps" : [ 0 ],
					"showheadphones" : [ 0 ],
					"showlegend" : [ 1 ],
					"showlistener" : [ 1 ],
					"showradius" : [ 0 ],
					"showshoebox" : [ 0 ],
					"showsources" : [ 1 ],
					"showsourceslabel" : [ 1 ],
					"showsourceslevels" : [ 0 ],
					"showspeakers" : [ 1 ],
					"showspeakershull" : [ 0 ],
					"showspeakerslabel" : [ 1 ],
					"showspeakerslevels" : [ 0 ],
					"showspeakersprojection" : [ 0 ],
					"showspeakersradius" : [ 0 ],
					"showspeakerstriangulation" : [ 0 ],
					"showviewer" : [ 1 ],
					"sourcecolor" : [ 0.490196, 1.0, 0.0, 1.0 ],
					"sourceproportion" : [ 0.07 ],
					"sourceseditable" : [ 1 ],
					"sourceslevels" : [ -60.0 ],
					"sourcespositions" : [ 0.097216, 2.991505, 0.04411 ],
					"speakerseditable" : [ 0 ],
					"speakerslevels" : [ -60.0, -60.0, -60.0, -60.0 ],
					"speakerspositions" : [ -0.712835, 0.701297, -0.007, 0.700936, 0.712733, 0.026486, 0.712835, -0.701297, 0.007, -0.700936, -0.712732, -0.026486 ],
					"speakersproportion" : [ 0.1 ],
					"useopengl" : [ 0 ],
					"viewpoint" : [ "top" ],
					"xoffset" : [ 0.0 ],
					"yaw" : [ -11.875732 ],
					"yoffset" : [ 0.0 ],
					"zoffset" : [ 0.0 ],
					"zoom" : [ 0.4 ],
					"zoomlock" : [ 0 ]
				}

			}
, 			{
				"box" : 				{
					"aperture" : [ 80.0 ],
					"aperturecolor" : [ 1.0, 1.0, 1.0, 0.62 ],
					"areasmonitoring" : [ 0 ],
					"autozoom" : [ 0 ],
					"backgroundcolor" : [ 0.7, 0.7, 0.7, 1.0 ],
					"backgroundimage" : [ "none" ],
					"backgroundimageangle" : [ 0.0 ],
					"backgroundimageopacity" : [ 1.0 ],
					"backgroundimagequality" : [ "medium" ],
					"backgroundimagescale" : [ 1.0 ],
					"backgroundimagexoffset" : [ 0.0 ],
					"backgroundimageyoffset" : [ 0.0 ],
					"circularconstraint" : [ 1 ],
					"dashedgrid" : [ 0 ],
					"defer" : [ 0 ],
					"display" : [ 1 ],
					"format" : [ "aed" ],
					"globalproportion" : [ 0.1 ],
					"gridcolor" : [ 1.0, 1.0, 1.0, 1.0 ],
					"gridlines" : [ 3 ],
					"gridmode" : [ "circular" ],
					"gridspacing" : [ 1.0 ],
					"id" : "obj-7",
					"jspainterfile" : [ "" ],
					"layout" : [ "leftright" ],
					"listenereditable" : [ 0 ],
					"listenerpitch" : [ 0.0 ],
					"listenerposition" : [ 0.0, 0.0, 0.0 ],
					"listenerproportion" : [ 0.15 ],
					"listenerroll" : [ 0.0 ],
					"listeneryaw" : [ 0.0 ],
					"maxclass" : "spat.viewer.embedded",
					"name" : [ "1" ],
					"numanchors" : [ 0 ],
					"numangulardivisions" : [ 8 ],
					"numareas" : [ 0 ],
					"numinlets" : 1,
					"numoutlets" : 7,
					"numsources" : [ 1 ],
					"numspeakers" : [ 4 ],
					"orientationmode" : [ "yawconstraint" ],
					"outlettype" : [ "source", "speakers", "source", "", "listener", "", "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 140.0, 220.0, 448.0, 238.0 ],
					"presentation" : 1,
					"presentation_rect" : [ 74.0, 160.0, 466.0, 500.0 ],
					"radius" : [ 1.0 ],
					"radiusconstraint" : [ 0 ],
					"rightclicklock" : [ 0 ],
					"shoeboxcorners" : [ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 ],
					"showanchors" : [ 0 ],
					"showanchorslabel" : [ 0 ],
					"showangulardivisions" : [ 0 ],
					"showaperture" : [ 0 ],
					"showareas" : [ 0 ],
					"showbackgroundimage" : [ 1 ],
					"showfps" : [ 0 ],
					"showheadphones" : [ 0 ],
					"showlegend" : [ 1 ],
					"showlistener" : [ 1 ],
					"showradius" : [ 0 ],
					"showshoebox" : [ 0 ],
					"showsources" : [ 1 ],
					"showsourceslabel" : [ 1 ],
					"showsourceslevels" : [ 0 ],
					"showspeakers" : [ 1 ],
					"showspeakershull" : [ 0 ],
					"showspeakerslabel" : [ 1 ],
					"showspeakerslevels" : [ 0 ],
					"showspeakersprojection" : [ 0 ],
					"showspeakersradius" : [ 0 ],
					"showspeakerstriangulation" : [ 0 ],
					"showviewer" : [ 1 ],
					"sourcecolor" : [ 0.490196, 1.0, 0.0, 1.0 ],
					"sourceproportion" : [ 0.07 ],
					"sourceseditable" : [ 1 ],
					"sourceslevels" : [ -60.0 ],
					"sourcespositions" : [ 0.12242, 2.990905, 0.0 ],
					"speakerseditable" : [ 0 ],
					"speakerslevels" : [ -60.0, -60.0, -60.0, -60.0 ],
					"speakerspositions" : [ -0.707107, 0.707107, 0.0, 0.707107, 0.707107, 0.0, 0.707107, -0.707107, 0.0, -0.707107, -0.707107, 0.0 ],
					"speakersproportion" : [ 0.1 ],
					"useopengl" : [ 0 ],
					"viewpoint" : [ "top" ],
					"xoffset" : [ 0.0 ],
					"yaw" : [ 0.0 ],
					"yoffset" : [ 0.0 ],
					"zoffset" : [ 0.0 ],
					"zoom" : [ 0.4 ],
					"zoomlock" : [ 0 ]
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 11.0,
					"id" : "obj-9",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "source" ],
					"patching_rect" : [ 140.0, 474.0, 29.0, 21.0 ],
					"style" : "",
					"text" : "thru"
				}

			}
, 			{
				"box" : 				{
					"angle" : 0.0,
					"bgcolor" : [ 0.666667, 0.666667, 0.666667, 0.5 ],
					"id" : "obj-47",
					"maxclass" : "panel",
					"mode" : 0,
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 610.0, 223.75, 220.0, 91.5 ],
					"presentation" : 1,
					"presentation_rect" : [ 320.0, 31.0, 220.0, 91.5 ],
					"proportion" : 0.39,
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-5",
					"maxclass" : "newobj",
					"numinlets" : 4,
					"numoutlets" : 4,
					"outlettype" : [ "", "", "", "" ],
					"patching_rect" : [ 632.0, 128.0, 121.0, 22.0 ],
					"style" : "",
					"text" : "route /yaw /pitch /roll"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-2",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"patching_rect" : [ 632.0, 82.0, 109.0, 22.0 ],
					"style" : "",
					"text" : "OSC-route /hedrot"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-1",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 632.0, 42.0, 99.0, 22.0 ],
					"style" : "",
					"text" : "udpreceive 2001"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"destination" : [ "obj-2", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-1", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-42", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"midpoints" : [ 325.333344, 553.0, 149.5, 553.0 ],
					"source" : [ "obj-10", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-14", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-11", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-42", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"midpoints" : [ 29.5, 553.0, 149.5, 553.0 ],
					"order" : 0,
					"source" : [ "obj-12", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-7", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"order" : 1,
					"source" : [ "obj-12", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-7", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-14", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 1 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-15", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-15", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-6", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-17", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-17", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-18", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-6", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-19", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-5", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-2", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-19", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-20", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-6", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-25", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-18", 2 ],
					"disabled" : 0,
					"hidden" : 0,
					"order" : 0,
					"source" : [ "obj-33", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-34", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"order" : 1,
					"source" : [ "obj-33", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-48", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"midpoints" : [ 774.5, 318.875, 634.5, 318.875 ],
					"source" : [ "obj-34", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-18", 1 ],
					"disabled" : 0,
					"hidden" : 0,
					"order" : 0,
					"source" : [ "obj-35", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-36", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"order" : 1,
					"source" : [ "obj-35", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-48", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"midpoints" : [ 704.5, 318.875, 634.5, 318.875 ],
					"source" : [ "obj-36", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-18", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"order" : 0,
					"source" : [ "obj-37", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-38", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"order" : 1,
					"source" : [ "obj-37", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-48", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"midpoints" : [ 634.5, 318.875, 634.5, 318.875 ],
					"source" : [ "obj-38", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-6", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-4", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-42", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-40", 2 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-42", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-40", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-42", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-40", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-3", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-42", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-9", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"midpoints" : [ 634.5, 461.375, 149.5, 461.375 ],
					"source" : [ "obj-48", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-33", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-5", 2 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-35", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-5", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-37", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-5", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-15", 1 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-6", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-15", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-6", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-10", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-7", 5 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-6", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"order" : 0,
					"source" : [ "obj-7", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-9", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"midpoints" : [ 435.5, 461.5, 149.5, 461.5 ],
					"source" : [ "obj-7", 4 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-9", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"midpoints" : [ 221.0, 461.5, 149.5, 461.5 ],
					"source" : [ "obj-7", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-9", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"midpoints" : [ 149.5, 461.5, 149.5, 461.5 ],
					"order" : 1,
					"source" : [ "obj-7", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-12", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-8", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-40", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-9", 0 ]
				}

			}
 ],
		"parameters" : 		{
			"obj-25::obj-74" : [ "live.tab[1]", "live.tab", 0 ],
			"obj-4::obj-6.1::obj-9::obj-26::obj-58" : [ "live.toggle[1]", "live.toggle[1]", 0 ],
			"obj-4::obj-6.1::obj-9::obj-43::obj-8" : [ "live.dial", "freq", 0 ],
			"obj-4::obj-6.1::obj-9::obj-182" : [ "live.text[1]", "live.text[1]", 0 ],
			"obj-25::obj-76" : [ "live.tab[2]", "live.tab", 0 ],
			"obj-4::obj-6.1::obj-9::obj-5::obj-12" : [ "live.button", "live.button", 0 ],
			"obj-4::obj-6.1::obj-9::obj-26::obj-56" : [ "live.button[1]", "live.button[1]", 0 ],
			"obj-25::obj-14" : [ "live.menu[1]", "live.menu", 0 ],
			"obj-4::obj-6.1::obj-9::obj-26::obj-59" : [ "live.numbox[1]", "live.numbox[1]", 0 ],
			"obj-25::obj-53" : [ "live.tab", "live.tab", 0 ],
			"obj-4::obj-6.1::obj-9::obj-2" : [ "live.gain~", " ", 0 ],
			"obj-4::obj-6.1::obj-9::obj-12::obj-4" : [ "live.numbox", "live.numbox", 0 ],
			"obj-4::obj-6.1::obj-9::obj-23" : [ "live.toggle", "live.toggle", 0 ],
			"obj-4::obj-6.1::obj-9::obj-1" : [ "live.menu", "live.menu", 0 ],
			"obj-15" : [ "live.gain~[1]", "live.gain~[1]", 0 ],
			"obj-4::obj-6.1::obj-9::obj-46::obj-36" : [ "select folder[1]", "select folder", 0 ],
			"obj-4::obj-6.1::obj-9::obj-46::obj-6" : [ "live.text[3]", "live.text[1]", 0 ]
		}
,
		"dependency_cache" : [ 			{
				"name" : "thru.maxpat",
				"bootpath" : "C74:/patchers/m4l/Pluggo for Live resources/patches",
				"type" : "JSON",
				"implicit" : 1
			}
, 			{
				"name" : "spat.multiinputs~.maxpat",
				"bootpath" : "~/Documents/Max 7/Packages/ircam-spat/patchers",
				"type" : "JSON",
				"implicit" : 1
			}
, 			{
				"name" : "spat.input.poly~.maxpat",
				"bootpath" : "~/Documents/Max 7/Packages/ircam-spat/patchers",
				"type" : "JSON",
				"implicit" : 1
			}
, 			{
				"name" : "spat.input~.maxpat",
				"bootpath" : "~/Documents/Max 7/Packages/ircam-spat/patchers",
				"type" : "JSON",
				"implicit" : 1
			}
, 			{
				"name" : "spat.hrtf.selection.maxpat",
				"bootpath" : "~/Documents/Max 7/Packages/ircam-spat/patchers",
				"type" : "JSON",
				"implicit" : 1
			}
, 			{
				"name" : "OSC-route.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "spat.viewer.embedded.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "spat.transform.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "spat.pan~.mxo",
				"type" : "iLaX"
			}
 ],
		"autosave" : 0
	}

}

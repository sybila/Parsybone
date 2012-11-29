TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += \
    ../main.cpp \
    ../sqlite/sqlite3.c

QMAKE_CXXFLAGS += -std=gnu++0x

unix:!macx:!symbian: LIBS += -lpthread -ldl

HEADERS += \
    ../auxiliary/user_options.hpp \
    ../auxiliary/time_manager.hpp \
    ../auxiliary/output_streamer.hpp \
    ../auxiliary/data_types.hpp \
    ../auxiliary/common_functions.hpp \
    ../construction/product_structure.hpp \
    ../construction/product_builder.hpp \
    ../construction/parametrizations_holder.hpp \
    ../construction/parametrizations_builder.hpp \
    ../construction/labeling_holder.hpp \
    ../construction/labeling_builder.hpp \
    ../construction/graph_interface.hpp \
    ../construction/construction_manager.hpp \
    ../construction/construction_holder.hpp \
    ../construction/basic_structure_builder.hpp \
    ../construction/basic_structure.hpp \
    ../construction/automaton_structure.hpp \
    ../construction/automaton_interface.hpp \
    ../construction/automaton_builder.hpp \
    ../parsing/rapidxml-1.13/rapidxml_utils.hpp \
    ../parsing/rapidxml-1.13/rapidxml_print.hpp \
    ../parsing/rapidxml-1.13/rapidxml_iterators.hpp \
    ../parsing/rapidxml-1.13/rapidxml.hpp \
    ../parsing/xml_helper.hpp \
    ../parsing/translator.hpp \
    ../parsing/time_series_parser.hpp \
    ../parsing/parsing_manager.hpp \
    ../parsing/network_parser.hpp \
    ../parsing/model_parser.hpp \
    ../parsing/model.hpp \
    ../parsing/formulae_parser.hpp \
    ../parsing/automaton_parser.hpp \
    ../parsing/argument_parser.hpp \
    ../synthesis/database_filler.hpp \
    ../synthesis/witness_searcher.hpp \
    ../synthesis/synthesis_manager.hpp \
    ../synthesis/split_manager.hpp \
    ../synthesis/robustness_compute.hpp \
    ../synthesis/paramset_helper.hpp \
    ../synthesis/output_manager.hpp \
    ../synthesis/model_checker.hpp \
    ../synthesis/color_storage.hpp \
    ../synthesis/coloring_analyzer.hpp \
    ../construction/unparametrized_structure.hpp \
    ../construction/unparametrized_structure_builder.hpp \
    ../synthesis/SQLAdapter.hpp \
    ../sqlite/sqlite3ext.h \
    ../sqlite/sqlite3.h \
    ../parsing/bitmask_manager.hpp \
    ../testing/tester.hpp \
    ../testing/includes.hpp

//Copyright 2015 Ryan Wick

//This file is part of Bandage

//Bandage is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//Bandage is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with Bandage.  If not, see <http://www.gnu.org/licenses/>.


#include "image.h"
#include "commoncommandlinefunctions.h"
#include "../program/globals.h"
#include "../ui/mygraphicsscene.h"
#include "../ui/mygraphicsview.h"
#include "../graph/assemblygraph.h"
#include <vector>
#include "../program/settings.h"
#include <QPainter>

int bandageImage(QStringList arguments)
{
    g_settings = new Settings();
    g_graphicsView = new MyGraphicsView();

    QTextStream out(stdout);
    QTextStream err(stdout);

    if (checkForHelp(arguments))
    {
        printImageUsage(&out);
        return 0;
    }

    if (arguments.size() < 2)
    {
        printImageUsage(&err);
        return 1;
    }

    QString graphFilename = arguments.at(0);
    arguments.pop_front();

    QString imageSaveFilename = arguments.at(0);
    arguments.pop_front();

    QString imageFileExtension = imageSaveFilename.right(4);
    if (imageFileExtension != ".png" && imageFileExtension != ".jpg")
    {
        err << "" << endl << "Error: the output filename must end in .png or .jpg" << endl;
        printImageUsage(&err);
        return 1;
    }

    QString error = checkForInvalidImageOptions(arguments);
    if (error.length() > 0)
    {
        err << "" << endl << "Error: " << error << endl;
        printImageUsage(&err);
        return 1;
    }

    bool loadSuccess = loadAssemblyGraph(graphFilename);
    if (!loadSuccess)
        return 1;

    MyGraphicsScene scene;

    int width = 0;
    int height = 0;
    parseImageOptions(arguments, &width, &height);

    //CURRENTLY FIXED AS WHOLE_GRAPH, THOUGH I WOULD LIKE TO ADD
    //SUPPORT FOR AROUND_BLAST_HITS
    g_settings->graphScope = WHOLE_GRAPH;
    int nodeDistance = 0;
    std::vector<DeBruijnNode *> startingNodes;

    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, nodeDistance);
    layoutGraph();

    g_assemblyGraph->addGraphicsItemsToScene(&scene);
    scene.setSceneRectangle();
    double sceneRectAspectRatio = scene.sceneRect().width() / scene.sceneRect().height();

    //Determine image size
    //If neither height nor width set, use a default of height = 1000.
    if (height == 0 && width == 0)
        height = 1000;

    //If only height or width is set, scale the other to fit.
    if (height > 0 && width == 0)
        width = height * sceneRectAspectRatio;
    else if (height == 0 && width > 0)
        height = width / sceneRectAspectRatio;

    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    scene.render(&painter);

    bool success = image.save(imageSaveFilename);
    if (!success)
    {
        out << "There was an error writing the image to file." << endl;
        return 1;
    }
    return 0;
}


void printImageUsage(QTextStream * out)
{
    *out << endl;
    *out << "Usage:   Bandage image <graphfile> <outputfile> [options]" << endl;
    *out << endl;
    *out << "Options: --height <int>      image height (default: 1000)" << endl;
    *out << "         --width <int>       image width (default: not set)" << endl;
    *out << "                             If only height or width is set, the other will be" << endl;
    *out << "                             determined automatically. If both are set, the" << endl;
    *out << "                             image will be exactly that size." << endl;
    *out << endl;
//           --------------------------------------------------------------------------------  //80 character guide
    *out << "         --double            draw graph in double mode (default: off)" << endl;
    *out << "         --bases <int>       base pairs per segment (default: auto)" << endl;
    *out << "                             High values result in longer nodes, small values in" << endl;
    *out << "                             shorter nodes." << endl;
    *out << "         --quality <int>     graph layout quality, 1 (low) to 5 (high)" << endl;
    *out << "                             (default: 3)" << endl;
    *out << endl;
//           --------------------------------------------------------------------------------  //80 character guide
    *out << "         --nodewidth <float> Average node width (0.5 to 1000, default: 5.0)" << endl;
    *out << "         --covwidth <float>  Coverage effect on width (0 to 1, default: 0.5)" << endl;
    *out << "         --covpower <float>  Power of coverage effect on width (0.1 to 1," << endl;
    *out << "                             default: 0.5)" << endl;
    *out << "                             Node widths are determined using the following" << endl;
    *out << "                             formula: a*b*((c/d)^e-1)+1" << endl;
    *out << "                                      a = average node width" << endl;
    *out << "                                      b = coverage effect on width" << endl;
    *out << "                                      c = node coverage" << endl;
    *out << "                                      d = mean coverage" << endl;
    *out << "                                      e = power of coverage effect on width" << endl;
    *out << endl;
//           --------------------------------------------------------------------------------  //80 character guide
    *out << "         --edgewidth <float> Edge width (0.1 to 1000, default: 2.0)" << endl;
    *out << "         --outline <float>   Node outline thickness (0 to 1000, default: 0.5) " << endl;
    *out << endl;
//           --------------------------------------------------------------------------------  //80 character guide
    *out << "         --names             Label nodes with name" << endl;
    *out << "         --lengths           Label nodes with length" << endl;
    *out << "         --coverages         Label nodes with coverage" << endl;
    *out << "         --fontsize <int>    Font size for node labels (1 to 100, default: 10)" << endl;
    *out << "         --toutline <float>  Surround text with a white outline with this" << endl;
    *out << "                             thickness (default: 0.3))" << endl;
    *out << endl;
//           --------------------------------------------------------------------------------  //80 character guide
    *out << "         --colour <scheme>   Node colouring scheme, choose one of the following" << endl;
    *out << "                             options: random, uniform or coverage (default:" << endl;
    *out << "                             random)" << endl;
    *out << "                             Specific colours can be set using HTML-style" << endl;
    *out << "                             hexidemical values." << endl;
    *out << "         --unicolpos <hex>   Node colour for positive nodes under the uniform" << endl;
    *out << "                             colouring scheme (default: " + g_settings->uniformPositiveNodeColour.name().right(6) + ")" << endl;
    *out << "         --unicolneg <hex>   Node colour for negative nodes under the uniform" << endl;
    *out << "                             colouring scheme (default: " + g_settings->uniformNegativeNodeColour.name().right(6) + ")" << endl;
    *out << "         --unicolspe <hex>   Node colour for special nodes under the uniform" << endl;
    *out << "                             colouring scheme (default: " + g_settings->uniformNodeSpecialColour.name().right(6) + ")" << endl;
    *out << endl;
}

QString checkForInvalidImageOptions(QStringList arguments)
{
    QString error = "";

    error = checkOptionForInt("--height", &arguments, 1, 32767);
    if (error.length() > 0) return error;
    error = checkOptionForInt("--width", &arguments, 1, 32767);
    if (error.length() > 0) return error;

    checkOptionWithoutValue("--double", &arguments);
    error = checkOptionForInt("--bases", &arguments, 1, std::numeric_limits<int>::max());
    if (error.length() > 0) return error;
    error = checkOptionForInt("--quality", &arguments, 1, 5);
    if (error.length() > 0) return error;

    error = checkOptionForFloat("--nodewidth", &arguments, 0.5, 1000);
    if (error.length() > 0) return error;
    error = checkOptionForFloat("--covwidth", &arguments, 0.0, 1.0);
    if (error.length() > 0) return error;
    error = checkOptionForFloat("--covpower", &arguments, 0.1, 1.0);
    if (error.length() > 0) return error;

    error = checkOptionForFloat("--edgewidth", &arguments, 0.1, 1000.0);
    if (error.length() > 0) return error;
    error = checkOptionForFloat("--outline", &arguments, 0.0, 1000.0);
    if (error.length() > 0) return error;

    checkOptionWithoutValue("--names", &arguments);
    checkOptionWithoutValue("--lengths", &arguments);
    checkOptionWithoutValue("--coverages", &arguments);
    error = checkOptionForInt("--fontsize", &arguments, 1, 100);
    if (error.length() > 0) return error;
    error = checkOptionForFloat("--toutline", &arguments, 0.0, 2.0);
    if (error.length() > 0) return error;

    QStringList validColourOptions;
    validColourOptions << "random" << "uniform" << "coverage";
    error = checkOptionForString("--colour", &arguments, validColourOptions);
    if (error.length() > 0) return error;

    error = checkOptionForHexColour("--unicolpos", &arguments);
    if (error.length() > 0) return error;
    error = checkOptionForHexColour("--unicolneg", &arguments);
    if (error.length() > 0) return error;
    error = checkOptionForHexColour("--unicolspe", &arguments);
    if (error.length() > 0) return error;

    return checkForExcessArguments(arguments);
}




//This function parses the command line options.  It assumes that the options
//have already been checked for correctness.
void parseImageOptions(QStringList arguments, int * width, int * height)
{
    if (isOptionPresent("--height", &arguments))
        *height = getIntOption("--height", &arguments);

    if (isOptionPresent("--width", &arguments))
        *width = getIntOption("--width", &arguments);

    g_settings->doubleMode = isOptionPresent("--double", &arguments);

    if (isOptionPresent("--bases", &arguments))
    {
        g_settings->manualBasePairsPerSegment = getIntOption("--bases", &arguments);
        g_settings->nodeLengthMode = MANUAL_NODE_LENGTH;
    }

    if (isOptionPresent("--quality", &arguments))
    {
        int quality = getIntOption("--quality", &arguments);
        if (quality < 0)
            quality = 0;
        if (quality > 4)
            quality = 4;
        g_settings->graphLayoutQuality = quality;
    }

    if (isOptionPresent("--nodewidth", &arguments))
        g_settings->averageNodeWidth = getFloatOption("--nodewidth", &arguments);

    if (isOptionPresent("--covwidth", &arguments))
        g_settings->coverageEffectOnWidth = getFloatOption("--covwidth", &arguments);

    if (isOptionPresent("--covpower", &arguments))
        g_settings->coveragePower = getFloatOption("--covpower", &arguments);

    if (isOptionPresent("--edgewidth", &arguments))
        g_settings->edgeWidth = getFloatOption("--edgewidth", &arguments);

    if (isOptionPresent("--outline", &arguments))
        g_settings->outlineThickness = getFloatOption("--outline", &arguments);
    else
        g_settings->outlineThickness = 0.5;

    g_settings->displayNodeNames = isOptionPresent("--names", &arguments);

    g_settings->displayNodeLengths = isOptionPresent("--lengths", &arguments);

    g_settings->displayNodeCoverages = isOptionPresent("--coverages", &arguments);

    QFont font = g_settings->labelFont;
    int fontsize = 10;
    if (isOptionPresent("--fontsize", &arguments))
        fontsize = getIntOption("--fontsize", &arguments);
    font.setPointSize(fontsize);
    g_settings->labelFont = font;

    if (isOptionPresent("--toutline", &arguments))
    {
        double textOutlineThickness = getFloatOption("--toutline", &arguments);
        if (textOutlineThickness == 0.0)
            g_settings->textOutline = false;
        else
        {
            g_settings->textOutline = true;
            g_settings->textOutlineThickness = textOutlineThickness;
        }
    }
    else
    {
        g_settings->textOutline = true;
        g_settings->textOutlineThickness = 0.3;
    }

    if (isOptionPresent("--colour", &arguments))
        g_settings->nodeColourScheme = getColourSchemeOption("--colour", &arguments);

    if (isOptionPresent("--unicolpos", &arguments))
        g_settings->uniformPositiveNodeColour = getHexColourOption("--unicolpos", &arguments);

    if (isOptionPresent("--unicolneg", &arguments))
        g_settings->uniformNegativeNodeColour = getHexColourOption("--unicolneg", &arguments);

    if (isOptionPresent("--unicolspe", &arguments))
        g_settings->uniformNodeSpecialColour = getHexColourOption("--unicolspe", &arguments);

}


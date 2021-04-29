#pragma once

#include <libfile/Archive.h>

#include <libwidget/Components.h>
#include <libwidget/Window.h>

#include <libfilepicker/FilePicker.h>
#include <libfilepicker/model/Navigation.h>
#include <libfilepicker/widgets/ArchiveBrowser.h>
#include <libfilepicker/widgets/JumpList.h>
#include <libfilepicker/widgets/ToolBar.h>

class MainWindow :
    public Widget::Window
{
private:
    RefPtr<FilePicker::Navigation> _navigation;
    RefPtr<Archive> _archive;
    FilePicker::Dialog _dialog;

public:
    void set_archive(RefPtr<Archive> archive)
    {
        _archive = archive;
        render();
    }

    MainWindow(RefPtr<FilePicker::Navigation> navigation, RefPtr<Archive> archive)
        : Widget::Window(WINDOW_RESIZABLE), _navigation(navigation), _archive(archive)
    {
        size(Math::Vec2i(700, 500));

        render();
    }

    void render()
    {
        root()->clear();
        root()->layout(VFLOW(0));

        root()->add(Widget::titlebar(Graphic::Icon::get("folder-zip"), "Archive Manager"));

        auto browser = root()->add<Widget::Element>();

        browser->flags(Widget::Element::FILL);

        if (!_archive)
        {
            browser->layout(STACK());
            auto load_button = browser->add(Widget::filled_button(Graphic::Icon::get("folder-open"), "Load an archive file", [&] {
                if (_dialog.show() == Widget::DialogResult::OK)
                {
                    set_archive(Archive::open(IO::Path::parse(_dialog.selected_file().unwrap())));
                }
            }));
        }
        else
        {
            browser->layout(VFLOW(0));

            auto toolbar = browser->add(Widget::panel());
            toolbar->layout(HFLOW(4));
            toolbar->insets(Insetsi(4, 4));

            toolbar->add(Widget::basic_button(Graphic::Icon::get("archive-arrow-up"), "Extract All"));

            toolbar->add(Widget::separator());

            auto load_button = toolbar->add(Widget::basic_button(Graphic::Icon::get("folder-open"), [&] {
                if (_dialog.show() == Widget::DialogResult::OK)
                {
                    set_archive(Archive::open(IO::Path::parse(_dialog.selected_file().unwrap())));
                }
            }));

            browser->add<FilePicker::ArchiveBrowser>(_navigation, _archive);
        }
    }
};

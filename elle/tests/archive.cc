#include <unordered_set>

#include <elle/archive/zip.hh>
#include <elle/attribute.hh>
#include <elle/filesystem.hh>
#include <elle/filesystem/TemporaryDirectory.hh>
#include <elle/finally.hh>
#include <elle/system/Process.hh>
#include <elle/test.hh>

using elle::filesystem::TemporaryDirectory;

class DummyHierarchy
{
public:
  DummyHierarchy()
  {
    auto pattern =
      boost::filesystem::temp_directory_path() / "%%%%-%%%%-%%%%-%%%%";
    this->_root = boost::filesystem::unique_path(pattern) / "root";
    auto root = this->_root;
    auto sub = root / "sub";
    boost::filesystem::create_directories(sub);
    boost::filesystem::ofstream(root / "1") << "1";
    boost::filesystem::ofstream(root / "2") << "2";
    boost::filesystem::ofstream(sub  / "3") << "3";
    boost::filesystem::ofstream(sub  / "4") << "4";
  }

  ~DummyHierarchy()
  {
    boost::filesystem::remove_all(this->_root.parent_path());
  }

  boost::filesystem::path
  root() const
  {
    return this->_root;
  }

private:
  boost::filesystem::path _root;
};

class TemporaryFile
{
public:
  TemporaryFile(std::string const& name)
    : _directory(name)
  {
    this->_path = this->_directory.path() / name;
    boost::filesystem::ofstream(this->_path);
  }

private:
  ELLE_ATTRIBUTE_R(TemporaryDirectory, directory);
  ELLE_ATTRIBUTE_R(boost::filesystem::path, path);
};

class ChangeDirectory
{
public:
  ChangeDirectory(boost::filesystem::path const& path)
    : _path(path)
    , _previous(boost::filesystem::current_path())
  {
    boost::filesystem::current_path(path);
  }

  ~ChangeDirectory()
  {
    boost::filesystem::current_path(this->_previous);
  }

private:
  ELLE_ATTRIBUTE_R(boost::filesystem::path, path);
  ELLE_ATTRIBUTE_R(boost::filesystem::path, previous);
};

static
void
check_file_content(boost::filesystem::path const& path,
                   char content)
{
  char result[BUFSIZ];
  boost::filesystem::ifstream f(path);
  f.read(result, BUFSIZ);
  BOOST_CHECK_EQUAL(f.gcount(), 1);
  BOOST_CHECK_EQUAL(result[0], content);
}

static
boost::filesystem::path
renamer_forbid(boost::filesystem::path const& input)
{
  BOOST_FAIL("no file renaming needed");
  return input;
}

static
void
extract(elle::archive::Format fmt,
        boost::filesystem::path const& path,
        boost::filesystem::path const& where)
{
  ChangeDirectory cd(where);
  switch (fmt)
  {
    case elle::archive::Format::tar:
    case elle::archive::Format::tar_bzip2:
    case elle::archive::Format::tar_gzip:
    {
      elle::system::Process p(
        (cd.previous() / "libarchive/bin/bsdtar").string(),
        {"-x", "-f", path.string()});
      BOOST_CHECK_EQUAL(p.wait_status(), 0);
      break;
    }
    case elle::archive::Format::zip:
    {
      elle::system::Process p(
        (cd.previous() / "libarchive/bin/bsdcpio").string(),
        {"--extract", "--make-directories", "-I", path.string()});
      BOOST_CHECK_EQUAL(p.wait_status(), 0);
      break;
    }
  }
}

static
void
archive(elle::archive::Format fmt)
{
  DummyHierarchy dummy;
  auto pattern =
    boost::filesystem::temp_directory_path() / "%%%%-%%%%-%%%%-%%%%.zip";
  auto path = boost::filesystem::unique_path(pattern);
  elle::SafeFinally clean([&] { boost::filesystem::remove(path); });
  elle::archive::archive(fmt, {dummy.root()}, path, renamer_forbid);
  {
    auto output_pattern =
      boost::filesystem::temp_directory_path() / "%%%%-%%%%-%%%%-%%%%";
    auto output = boost::filesystem::unique_path(output_pattern);
    boost::filesystem::create_directory(output);
    elle::SafeFinally clean([&] { boost::filesystem::remove_all(output); });
    extract(fmt, path, output);
    int count = 0;
    ChangeDirectory cd(output);
    for (auto it = boost::filesystem::recursive_directory_iterator(".");
         it != boost::filesystem::recursive_directory_iterator();
         ++it)
    {
      ++count;
      auto path = *it;
      if (path == "./root" || path == "./root/sub")
        continue;
      else if (path == "./root/1")
        check_file_content(path, '1');
      else if (path == "./root/2")
        check_file_content(path, '2');
      else if (path == "./root/sub/3")
        check_file_content(path, '3');
      else if (path == "./root/sub/4")
        check_file_content(path, '4');
      else
        BOOST_FAIL(path);
    }
    BOOST_CHECK_EQUAL(count, 6);
  }
}

static
boost::filesystem::path
renamer(boost::filesystem::path const& input)
{
  return input.string() + " bis";
}

static
void
archive_duplicate(elle::archive::Format fmt)
{
  TemporaryDirectory d1("same");
  boost::filesystem::ofstream(d1.path() / "beacon");
  TemporaryDirectory d2("same");
  boost::filesystem::ofstream(d2.path() / "beacon");
  TemporaryFile f("same");
  TemporaryDirectory output("output");
  auto path = output.path() / "output.zip";
  elle::archive::archive(fmt,
                         {d1.path(), d2.path(), f.path()},
                         path,
                         renamer);
  {
    TemporaryDirectory decompress("decompress");
    extract(fmt, path, decompress.path());
    int count = 0;
    std::unordered_set<boost::filesystem::path> accepted({
        "./same",
        "./same/beacon",
        "./same bis",
        "./same bis/beacon",
        "./same bis bis",
          });
    ChangeDirectory cd(decompress.path());
    for (auto it = boost::filesystem::recursive_directory_iterator(".");
         it != boost::filesystem::recursive_directory_iterator();
         ++it)
    {
      ++count;
      if (accepted.find(*it) == accepted.end())
        BOOST_FAIL(*it);
    }
    BOOST_CHECK_EQUAL(count, accepted.size());
  }
}

#define FORMAT(Fmt)                                     \
  namespace Fmt                                         \
  {                                                     \
    static                                              \
    void                                                \
    simple()                                            \
    {                                                   \
      archive(elle::archive::Format::Fmt);              \
    }                                                   \
                                                        \
    static                                              \
    void                                                \
    duplicate()                                         \
    {                                                   \
      archive_duplicate(elle::archive::Format::Fmt);    \
    }                                                   \
  }                                                     \

FORMAT(zip)
FORMAT(tar)
FORMAT(tar_gzip)
#undef FORMAT

ELLE_TEST_SUITE()
{
  auto& master = boost::unit_test::framework::master_test_suite();
#define FORMAT(Fmt)                             \
  {                                             \
    using namespace Fmt;                        \
    auto suite = BOOST_TEST_SUITE(#Fmt);        \
    master.add(suite);                          \
    suite->add(BOOST_TEST_CASE(simple));        \
    suite->add(BOOST_TEST_CASE(duplicate));     \
  }                                             \

  FORMAT(zip);
  FORMAT(tar);
  FORMAT(tar_gzip);
#undef FORMAT
}
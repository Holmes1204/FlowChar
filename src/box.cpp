#include <iostream>
#include <math.h>
#include <cassert>
#include <algorithm>

#include <box.h>

namespace FC { namespace BE {

Box::Box(const Kind kind) : kind(kind) {}

SeqBox::SeqBox() : Box(Kind::SEQ) {}

SimpleBox::SimpleBox(const std::string &content) : Box(Kind::SIMPLE), content(content) {}

IfBox::IfBox(const std::string &content, Box *const thent) : Box(Kind::IF), content(content), thent(thent), hasNext(false) {}

IfBox::IfBox(const std::string &content, Box *const thent, Box *const elsee) : Box(Kind::IF), content(content), thent(thent), elsee(elsee), hasNext(false) {}

WhileBox::WhileBox(const std::string &content, Box *const body) : Box(Kind::WHILE), content(content), body(body), hasNext(false) {}

AttachInfo SeqBox::Attach() {
    int maxLWidth = 0;
    int maxRWidth = 0;
    int height = 0;

    for (int i = 0; i < this->seq.size(); i++) {
        AttachInfo ainfo = this->seq[i]->Attach();
        maxLWidth = std::max(maxLWidth, ainfo.lWidth);
        maxRWidth = std::max(maxRWidth, ainfo.rWidth);
        
        // determine whether the if box points to a simple box or 'O' eventually
        if (i > 0 && this->seq[i-1]->kind == Kind::IF && this->seq[i]->kind == Kind::SIMPLE) {
            IfBox *ifBox = ((IfBox *)(this->seq[i - 1]));
            SimpleBox *simpleBox = ((SimpleBox *)(this->seq[i]));
            ifBox->hasNext = true;
            ifBox->height -= ((ifBox->hasElse) ? 2 : 3);
            height -= ((ifBox->hasElse) ? 2 : 3);

            // update width info if necessary
            if (!ifBox->hasElse) {
                if (ifBox->nSide)
                    ifBox->lWidth = std::max(ifBox->lWidth - 4, simpleBox->lWidth) + 4;
                else
                    ifBox->rWidth = std::max(ifBox->rWidth - 4, simpleBox->rWidth) + 4;
            }
            else {
                assert(simpleBox->lWidth == simpleBox->rWidth);
                int oldAxisDtstance = ifBox->axisDistance;
                ifBox->axisDistance = std::max(oldAxisDtstance, simpleBox->lWidth + 3);
                ifBox->lWidth += (ifBox->axisDistance - oldAxisDtstance);
                ifBox->rWidth += (ifBox->axisDistance - oldAxisDtstance);
            }

            maxLWidth = std::max(maxLWidth, ifBox->lWidth);
            maxRWidth = std::max(maxRWidth, ifBox->rWidth);
        }
        // determine whether the while box points to a simple box or 'O' eventually
        else if (i > 0 && this->seq[i-1]->kind == Kind::WHILE && this->seq[i]->kind == Kind::SIMPLE) {
            WhileBox *whileBox = ((WhileBox *)(this->seq[i - 1]));
            SimpleBox *simpleBox = ((SimpleBox *)(this->seq[i]));
            whileBox->hasNext = true;
            whileBox->height -= 3;
            height -= 3;

            // update width info if necessary
            whileBox->lWidth = std::max(whileBox->lWidth - 4, simpleBox->lWidth) + 4;

            maxLWidth = std::max(maxLWidth, whileBox->lWidth);
            maxRWidth = std::max(maxRWidth, whileBox->rWidth);
        }

        maxLWidth = std::max(maxLWidth, ainfo.lWidth);
        maxRWidth = std::max(maxRWidth, ainfo.rWidth);
        height += (ainfo.height + 2);
    }

    this->lWidth = maxLWidth;
    this->rWidth = maxRWidth;
    this->height = height - 2;
    return AttachInfo(this->lWidth, this->rWidth, this->height);
}

AttachInfo SimpleBox::Attach() {
    const int length = this->content.size();
    this->width = length + ((length % 2 == 0) ? 9 : 8);
    this->lWidth = (this->width - 1) / 2;
    this->rWidth = (this->width - 1) / 2;
    this->height = 3;

    return AttachInfo(this->lWidth, this->rWidth, this->height);
}

AttachInfo IfBox::Attach() {
    const int length = this->content.size();
    this->width = length + ((length % 2 == 0) ? 9 : 8);

    if (!this->elsee) {
        // only one side
        this->hasElse = false;
        AttachInfo ainfo = this->thent->Attach();
        
        // if left side is shorter, arrange 'no' branch at left to make flowchart more balanced
        this->nSide = (ainfo.lWidth < ainfo.rWidth);
        this->lWidth = std::max((this->width - 1) / 2, ainfo.lWidth) + (this->nSide ? 4 : 0);
        this->rWidth = std::max((this->width - 1) / 2, ainfo.rWidth) + (this->nSide ? 0 : 4);
        this->height = ainfo.height + 5 + 3;
    }
    else {
        // both sides
        this->hasElse = true;
        
        // 'yes' branch is at left side and 'no' branch is at right one
        AttachInfo lainfo = this->thent->Attach();
        AttachInfo rainfo = this->elsee->Attach();

        // fixed width of branch shoulder, must be int
        int minAxisDistance = (3 + this->width + 3 - 1) / 2;
        // fixed distance between both branches, may be float
        int actualAxisDistance = ceil((lainfo.rWidth + 3 + rainfo.lWidth - 1) / 2.0);
        this->axisDistance = std::max(actualAxisDistance, minAxisDistance);

        this->lWidth = this->axisDistance + lainfo.lWidth + 1;
        this->rWidth = this->axisDistance + rainfo.rWidth + 1;
        this->height = std::max(lainfo.height, rainfo.height) + 5 + 2;
    }
    return AttachInfo(this->lWidth, this->rWidth, this->height);
}

AttachInfo WhileBox::Attach() {
    const int length = this->content.size();
    this->width = length + ((length % 2 == 0) ? 9 : 8);
    AttachInfo ainfo = this->body->Attach();
    this->lWidth = std::max((this->width - 1) / 2, ainfo.lWidth) + 4;
    this->rWidth = std::max((this->width - 1) / 2, ainfo.rWidth) + 4;
    this->height = ainfo.height + 5 + 3;

    std::vector<Box *> seq = ((SeqBox *)(this->body))->seq;
    int size = seq.size();
    if ((size == 1 && seq[0]->kind == Kind::SIMPLE) ||
        (size > 1 && seq[size - 1]->kind == Kind::SIMPLE && seq[size - 2]->kind == Kind::SIMPLE) ||
        (size > 1 && seq[size - 1]->kind == Kind::SIMPLE && seq[size - 2]->kind == Kind::IF && ((IfBox *)(seq[size - 2]))->nSide)) {
        this->needExtraO = false;
    }
    else {
        this->needExtraO = true;
        this->height += 3;
    }

    return AttachInfo(this->lWidth, this->rWidth, this->height);
}

static void drawArrow(chartT &chart, const posT &from, const posT &to, const int vertical = -1) {
    // no turning point, vertical arrow, used in simple-box
    if (vertical < 0) {
        assert(from.second == to.second);
        for (int i = from.first; i < to.first; i++)
            chart[i][from.second] = '|';
        chart[to.first][from.second] = 'V';
        return;
    }

    // one turning point, used in if-box with else branch
    if (from.second == vertical) {
        // first vertical, then horizontal
        for (int i = from.first; i < to.first; i++)
            chart[i][vertical] = '|';
        for (int i = std::min(vertical, to.second); i <= std::max(vertical, to.second); i++)
            chart[to.first][i] = '-';
        chart[to.first][vertical] = '+';
        chart[to.first][to.second] = (vertical < to.second) ? '>' : '<';
        return;
    }
    if (to.second == vertical) {
        // first horizontal, then vertical
        for (int i = std::min(from.second, vertical); i <= std::max(from.second, vertical); i++)
            chart[from.first][i] = '-';
        chart[from.first][vertical] = '+';
        for (int i = from.first + 1; i < to.first; i++)
            chart[i][vertical] = '|';
        chart[to.first][to.second] = 'V';
        return;
    }

    // two turning points, used in if-box without else branch and while-box
    for (int i = std::min(from.second, vertical); i <= std::max(from.second, vertical); i++)
        chart[from.first][i] = '-';
    for (int i = std::min(from.first, to.first); i <= std::max(from.first, to.first); i++)
        chart[i][vertical] = '|';
    for (int i = std::min(to.second, vertical); i <= std::max(to.second, vertical); i++)
        chart[to.first][i] = '-';
    chart[from.first][vertical] = '+';
    chart[to.first][vertical] = '+';
    chart[to.first][to.second] = (vertical < to.second) ? '>' : '<';
    return;
}

static void printHelper(chartT &chart) {
    for (std::vector<char> row : chart) {
        for (char c : row) {
            std::cout << c;
        }
        std::cout << std::endl;
    }
    std::cout << "-------------------------------------" << std::endl << std::endl;
}

DrawInfo SeqBox::Draw(chartT &chart, const posT &pos) {
    int row = pos.first;
    const int col = pos.second;
    for (int i = 0; i < this->seq.size(); i++)
    {
        // printHelper(chart);
        DrawInfo dinfo = this->seq[i]->Draw(chart, std::make_pair(row, col));
        row += dinfo.height;
        // printHelper(chart);

        if (i == this->seq.size() - 1)
            return DrawInfo();  // return value from SeqBox::Draw doesn't make sense

        // draw arrow
        if (this->seq[i]->kind == Kind::SIMPLE) {
            drawArrow(chart, std::make_pair(row - 3, col), std::make_pair(row - 2, col));
        }
        else if (this->seq[i]->kind == Kind::IF) {
            IfBox *ifBox = (IfBox *)(this->seq[i]);
            if (!ifBox->hasNext) {
                drawArrow(chart, std::make_pair(row - 3, col), std::make_pair(row - 2, col));
                continue;
            }
            assert(this->seq[i + 1]->kind == Kind::SIMPLE);
            const int nextHalfWidth = (this->seq[i + 1]->width - 1) / 2;
            if (!ifBox->hasElse) {
                drawArrow(chart, std::make_pair(row - 3, col), std::make_pair(row - 2, col));  // vertical arrow
                if (ifBox->nSide) {
                    drawArrow(chart, dinfo.arrowAFrom, std::make_pair(row, col - nextHalfWidth - 1), col - this->seq[i]->lWidth);
                    chart[dinfo.arrowAFrom.first - 1][dinfo.arrowAFrom.second - 1] = 'N';
                }
                else {
                    drawArrow(chart, dinfo.arrowAFrom, std::make_pair(row, col + nextHalfWidth + 1), col + this->seq[i]->rWidth);
                    chart[dinfo.arrowAFrom.first - 1][dinfo.arrowAFrom.second + 1] = 'N';
                }
            }
            else {
                drawArrow(chart, dinfo.arrowAFrom, std::make_pair(row, col - nextHalfWidth - 1), dinfo.arrowAFrom.second);
                drawArrow(chart, dinfo.arrowBFrom, std::make_pair(row, col + nextHalfWidth + 1), dinfo.arrowBFrom.second);
            }
        }
        else if (this->seq[i]->kind == Kind::WHILE) {
            WhileBox *whileBox = (WhileBox *)(this->seq[i]);
            if (!whileBox->hasNext) {
                drawArrow(chart, std::make_pair(row - 3, col), std::make_pair(row - 2, col));
                continue;
            }
            assert(this->seq[i + 1]->kind == Kind::SIMPLE);
            const int nextHalfWidth = (this->seq[i + 1]->width - 1) / 2;
            drawArrow(chart, dinfo.arrowAFrom, std::make_pair(row, col - nextHalfWidth - 1), col - this->seq[i]->lWidth);
            chart[dinfo.arrowAFrom.first - 1][dinfo.arrowAFrom.second - 1] = 'N';
        }
        else {
            assert(0);
        }
    }
}

static void drawSingleBox(chartT &chart, const posT &pos, const int halfWidth, const std::string content, const bool isSimple) {
    // border
    chart[pos.first - 1][pos.second - halfWidth] = isSimple ? '+' : '/';
    chart[pos.first - 1][pos.second + halfWidth] = isSimple ? '+' : '\\';
    chart[pos.first][pos.second - halfWidth] = '|';
    chart[pos.first][pos.second + halfWidth] = '|';
    chart[pos.first + 1][pos.second - halfWidth] = isSimple ? '+' : '\\';
    chart[pos.first + 1][pos.second + halfWidth] = isSimple ? '+' : '/';
    for (int i = pos.second - halfWidth + 1; i < pos.second + halfWidth; i++) {
        chart[pos.first - 1][i] = '-';
        chart[pos.first + 1][i] = '-';
    }

    // content
    int length = content.size();
    int start = pos.second - (length - 1) / 2;
    for (int i = start; i < start + length; i++) {
        chart[pos.first][i] = content[i - start];
    }
}

DrawInfo SimpleBox::Draw(chartT &chart, const posT &pos) {
    drawSingleBox(chart, pos, (this->width - 1) / 2, this->content, true);
    return DrawInfo(this->height + 2);
}

DrawInfo IfBox::Draw(chartT &chart, const posT &pos) {
    drawSingleBox(chart, pos, (this->width - 1) / 2, this->content, false);

    // branch
    if (!this->hasElse) {
        drawArrow(chart, std::make_pair(pos.first + 2, pos.second), std::make_pair(pos.first + 3, pos.second));  // vertical
        chart[pos.first + 2][pos.second + 2] = 'Y';

        this->thent->Draw(chart, std::make_pair(pos.first + 5, pos.second));

        const int arrowAFromY = (this->nSide) ? (pos.second - (this->width - 1) / 2 - 1) : (pos.second + (this->width - 1) / 2 + 1);
        const posT arrowAFrom = std::make_pair(pos.first, arrowAFromY);

        if (this->hasNext) {
            return DrawInfo(this->height + 2, arrowAFrom);
        }
        else {
            const posT OPos = std::make_pair(pos.first + this->height - 2, pos.second);
            drawArrow(chart, std::make_pair(OPos.first - 2, OPos.second), std::make_pair(OPos.first - 1, OPos.second));  // vertical
            chart[OPos.first][OPos.second] = 'O';
            if (this->nSide) {
                chart[arrowAFrom.first - 1][arrowAFromY - 1] = 'N';
                drawArrow(chart, arrowAFrom, std::make_pair(OPos.first, OPos.second - 1), pos.second - this->lWidth);
            }
            else {
                chart[arrowAFrom.first - 1][arrowAFromY + 1] = 'N';
                drawArrow(chart, arrowAFrom, std::make_pair(OPos.first, OPos.second + 1), pos.second + this->rWidth);
            }
            return DrawInfo(this->height + 2);
        }
    }
    else {
        int leftAxis = pos.second - this->axisDistance - 1;
        int rightAxis = pos.second + this->axisDistance + 1;
        int halfWidth = (this->width - 1) / 2;
        drawArrow(chart, std::make_pair(pos.first, pos.second - halfWidth - 1), std::make_pair(pos.first + 3, leftAxis), leftAxis);
        chart[pos.first - 1][pos.second - halfWidth - 2] = 'Y';
        drawArrow(chart, std::make_pair(pos.first, pos.second + halfWidth + 1), std::make_pair(pos.first + 3, rightAxis), rightAxis);
        chart[pos.first - 1][pos.second + halfWidth + 2] = 'N';

        this->thent->Draw(chart, std::make_pair(pos.first + 5, leftAxis));
        this->elsee->Draw(chart, std::make_pair(pos.first + 5, rightAxis));

        const posT arrowAFrom = std::make_pair(pos.first + this->thent->height + 4, leftAxis);
        const posT arrowBFrom = std::make_pair(pos.first + this->elsee->height + 4, rightAxis);

        if (this->hasNext) {
            return DrawInfo(this->height + 2, arrowAFrom, arrowBFrom);
        }
        else {
            const posT OPos = std::make_pair(pos.first + this->height - 2, pos.second);
            drawArrow(chart, arrowAFrom, std::make_pair(OPos.first, OPos.second - 1), leftAxis);
            drawArrow(chart, arrowBFrom, std::make_pair(OPos.first, OPos.second + 1), rightAxis);
            chart[OPos.first][OPos.second] = 'O';
            return DrawInfo(this->height + 2);
        }
    }
    assert(0);
}

DrawInfo WhileBox::Draw(chartT &chart, const posT &pos) {
    drawSingleBox(chart, pos, (this->width - 1) / 2, this->content, false);

    // draw yes-branch arrow
    drawArrow(chart, std::make_pair(pos.first + 2, pos.second), std::make_pair(pos.first + 3, pos.second));  // vertical
    chart[pos.first + 2][pos.second + 2] = 'Y';

    // draw loop body
    this->body->Draw(chart, std::make_pair(pos.first + 5, pos.second));

    // draw backward arrow
    const posT backArrowTo = std::make_pair(pos.first, pos.second + (this->width - 1) / 2 + 1);
    if (!this->needExtraO) {
        Box *lastBox = ((SeqBox *)(this->body))->seq.back();
        assert(lastBox->kind == Kind::SIMPLE);
        const int lastBoxHalfWidth = (((SimpleBox *)(lastBox))->width - 1) / 2;
        const posT backArrowFrom = std::make_pair(pos.first + this->height - (this->hasNext ? 3 : 6), pos.second + lastBoxHalfWidth + 1);
        drawArrow(chart, backArrowFrom, backArrowTo, pos.second + this->rWidth);
    }
    else {
        const posT OPos = std::make_pair(pos.first + this->height - (this->hasNext ? 2 : 5), pos.second);
        drawArrow(chart, std::make_pair(OPos.first - 2, OPos.second), std::make_pair(OPos.first - 1, OPos.second));  // vertical
        chart[OPos.first][OPos.second] = 'O';
        drawArrow(chart, std::make_pair(OPos.first, OPos.second + 1), backArrowTo, pos.second + this->rWidth);
    }

    // draw no-branch arrow
    const posT arrowAFrom = std::make_pair(pos.first, pos.second - (this->width - 1) / 2 - 1);
    if (this->hasNext) {
        return DrawInfo(this->height + 2, arrowAFrom);
    }
    else {
        const posT OPos = std::make_pair(pos.first + this->height - 2, pos.second);
        chart[arrowAFrom.first - 1][arrowAFrom.second - 1] = 'N';
        drawArrow(chart, arrowAFrom, std::make_pair(OPos.first, OPos.second - 1), pos.second - this->lWidth);
        chart[OPos.first][OPos.second] = 'O';
        return DrawInfo(this->height + 2);
    }
    assert(0);
}

void SeqBox::Print(int d) const {
    std::cout << "---H: " << this->height << std::endl;
    for (Box *box : this->seq)
    {
        box->Print(d);
    }
}

static void indentHelper(int num) {
    for (int i = 0; i < num; i++) {
        std::cout << ' ';
    }
}

void SimpleBox::Print(int d) const {
    indentHelper(2 * d);
    std::cout << this->content << "  (W: " << this->width
     << ", L: " << this->lWidth
     << ", R: " << this->rWidth 
     << ", H: " << this->height << ")" << std::endl;
}

void IfBox::Print(int d) const {
    indentHelper(2 * d);
    std::cout << "if " << this->content << "  (W: " << this->width
     << ", L: " << this->lWidth
     << ", R: " << this->rWidth 
     << ", H: " << this->height
     << ", AD: " << this->axisDistance << ")" << std::endl;
    this->thent->Print(d + 1);
    if (this->elsee) {
        indentHelper(2 * d);
        std::cout << "else" << std::endl;
        this->elsee->Print(d + 1);
    }
}

void WhileBox::Print(int d) const {
    indentHelper(2 * d);
    std::cout << "while " << this->content << "  (W: " << this->width
     << ", L: " << this->lWidth
     << ", R: " << this->rWidth 
     << ", H: " << this->height << ")" << std::endl;
    this->body->Print(d + 1);
}

} // namespace BE
} // namespace FC